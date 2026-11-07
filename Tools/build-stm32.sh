#!/bin/sh
#
# Build the STM32 firmware outside Eclipse, with the flags the Eclipse project
# files specify.  Tools/cproject.awk reads Source/<family>/.cproject, so the
# compiler options, defines, include paths and linker script of every
# configuration come from the project itself and are never duplicated here.
#
# Usage:
#   Tools/build-stm32.sh [family | configuration ...]
#
#   no argument         build every STM32 configuration
#   S4xxxx              build every configuration of that project
#   S4SMC14             build that one configuration
#
# Environment:
#   CROSS    toolchain prefix, default arm-none-eabi-
#   OUT      output directory, default <repo>/build
#   STRICT   if 1, a compiler warning fails the build too
#
# Per configuration it writes <OUT>/<cfg>.elf, <OUT>/hex/<cfg>.hex and
# <OUT>/<cfg>.log, and one flash/RAM usage line to <OUT>/summary.txt.
# The exit status is non-zero if any configuration failed.

set -u

ROOT=$(cd "$(dirname "$0")/.." && pwd)
SRC=$ROOT/Source
OUT=${OUT:-$ROOT/build}
CROSS=${CROSS:-arm-none-eabi-}
STRICT=${STRICT:-0}

CC=${CROSS}gcc
OBJCOPY=${CROSS}objcopy
SIZE=${CROSS}size

command -v "$CC" >/dev/null 2>&1 || {
    echo "$CC not found in PATH" >&2
    exit 2
}

# Resolve one path as .cproject writes it.  The projects reach the shared code
# through the linked resources declared in .project:
#   Common      -> Source/Common
#   HAL         -> Source/HAL/STM32
#   HAL/CMSIS   -> Source/HAL/ARM/CMSIS
#   HAL/Device  -> Source/HAL/ARM/Device/ST/<family>   ($2, read from .project)
resolve() {
    p=$(printf '%s' "$1" | sed 's/&quot;//g; s|${workspace_loc:/${ProjName}/||; s|}$||')
    case "$p" in
        HAL/Device*) printf '%s%s\n' "$2" "$(printf '%s' "$p" | sed 's|^HAL/Device||')" ;;
        HAL/CMSIS*)  printf '%s%s\n' "$SRC/HAL/ARM/CMSIS" "$(printf '%s' "$p" | sed 's|^HAL/CMSIS||')" ;;
        HAL*)        printf '%s%s\n' "$SRC/HAL/STM32" "$(printf '%s' "$p" | sed 's|^HAL||')" ;;
        *)           printf '%s/%s\n' "$SRC" "$p" ;;
    esac
}

# Size of a region of the linker script, in bytes.  The scripts already hold
# back the pages used for EEPROM emulation, so these limits are the usable
# size and the linker itself rejects an overflow; this is for the report only.
ldsize() {
    awk -v r="$2" '
        /^[ \t]*MEMORY/ { m = 1 }
        m && $0 ~ "^[ \t]*" r "[ \t]*\\(" {
            if (match($0, /LENGTH[ \t]*=[ \t]*[0-9]+[KMkm]?/)) {
                s = substr($0, RSTART, RLENGTH); sub(/.*=[ \t]*/, "", s)
                n = s + 0
                if (s ~ /[Kk]$/) n *= 1024
                else if (s ~ /[Mm]$/) n *= 1048576
                print n; exit
            }
        }' "$1"
}

pct() {   # $1 used, $2 limit
    if [ -n "$2" ] && [ "$2" -gt 0 ] 2>/dev/null; then
        printf '%s/%s (%s%%)' "$1" "$2" "$(( $1 * 100 / $2 ))"
    else
        printf '%s' "$1"
    fi
}

all_families() {
    for d in "$SRC"/S[0-9]xxxx; do
        [ -f "$d/.cproject" ] && basename "$d"
    done
}

# An argument may name a project or a single configuration; collect both.
FAMS=
ONLY=
if [ $# -eq 0 ]; then
    FAMS=$(all_families)
else
    for a in "$@"; do
        if [ -f "$SRC/$a/.cproject" ]; then
            FAMS="$FAMS $a"
        else
            ONLY="$ONLY $a "
            FAMS="$FAMS $(all_families)"
        fi
    done
fi
FAMS=$(printf '%s\n' $FAMS | sort -u)

mkdir -p "$OUT/hex"
: > "$OUT/summary.txt"
rc=0
built=0
failed=0

for fam in $FAMS; do
    proj=$SRC/$fam
    dev=$(awk '/<name>HAL\/Device<\/name>/ { f = 1 }
               f && /locationURI/ { sub(/.*%7D\//, ""); sub(/<\/locationURI>.*/, ""); print; exit }' \
          "$proj/.project")
    DEV=$SRC/$dev

    # The vendor trees are inconsistent about the case of this directory.
    devsrc=
    for d in "$DEV/source" "$DEV/Source"; do
        [ -d "$d" ] && devsrc=$d
    done
    if [ -z "$devsrc" ]; then
        echo "$fam: no device sources under $DEV" >&2
        rc=1
        continue
    fi

    opt=$OUT/$fam.opt
    awk -f "$ROOT/Tools/cproject.awk" "$proj/.cproject" > "$opt"

    for cfg in $(cut -f1 "$opt" | sort -u); do
        if [ -n "$ONLY" ]; then
            case "$ONLY" in
                *" $cfg "*) ;;
                *) continue ;;
            esac
        fi

        one=$OUT/.opt
        grep "^$cfg	" "$opt" > "$one"
        get() {
            awk -F'\t' -v k="ilg.gnuarmeclipse.managedbuild.cross.option.$1" \
                '$3 == k { print $4 }' "$one"
        }

        defs=
        for d in $(get c.compiler.defs); do
            defs="$defs -D$d"
        done

        incs=
        for i in $(get c.compiler.include.paths); do
            incs="$incs -I$(resolve "$i" "$DEV")"
        done

        ld=$(resolve "$(get c.linker.scriptfile | head -1)" "$DEV")
        mcpu=$(get arm.target.family   | sed 's/.*mcpu\.//')
        olvl=$(get optimization.level  | sed 's/.*level\.//')
        fpuu=$(get arm.target.fpu.unit | sed 's/.*unit\.//')
        fpua=$(get arm.target.fpu.abi  | sed 's/.*abi\.//')

        case "$olvl" in
            none)     O=-O0 ;;
            optimize) O=-O1 ;;
            more)     O=-O2 ;;
            most)     O=-O3 ;;
            size)     O=-Os ;;
            debug)    O=-Og ;;
            *)        O=-O1 ;;
        esac

        fpu=
        [ "$fpuu" = "fpv4spd16" ] && fpu="-mfpu=fpv4-sp-d16"
        if [ -n "$fpua" ] && [ "$fpua" != "default" ]; then
            fpu="$fpu -mfloat-abi=$fpua"
        fi

        ARCH="-mcpu=$mcpu -mthumb $fpu"
        WARN="-pedantic -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -Wmissing-prototypes -Wstrict-prototypes -Wbad-function-cast"
        CFLAGS="$ARCH $O -fmessage-length=0 -ffunction-sections -fdata-sections $WARN -std=gnu11 $defs $incs"

        srcs=$(ls "$SRC"/Common/*.c "$SRC"/Common/EXT/*.c "$SRC"/Common/PHY/*/*.c \
                  "$SRC"/HAL/STM32/Driver/*.c "$devsrc"/*.c 2>/dev/null)
        asms=$(ls "$devsrc"/gcc/*.S 2>/dev/null)

        bdir=$OUT/obj/$cfg
        rm -rf "$bdir"
        mkdir -p "$bdir"
        log=$OUT/$cfg.log
        : > "$log"
        ok=1
        objs=

        for f in $srcs; do
            o=$bdir/$(basename "$f" .c).o
            $CC $CFLAGS -c -o "$o" "$f" >> "$log" 2>&1 || ok=0
            objs="$objs $o"
        done
        for f in $asms; do
            o=$bdir/$(basename "$f" .S).o
            $CC $ARCH $O -x assembler-with-cpp $defs -c -o "$o" "$f" >> "$log" 2>&1 || ok=0
            objs="$objs $o"
        done

        line=
        if [ $ok -eq 1 ]; then
            if $CC $ARCH $O -T"$ld" --specs=nano.specs -Wl,--gc-sections \
                   -o "$OUT/$cfg.elf" $objs -lm >> "$log" 2>&1
            then
                $OBJCOPY -O ihex "$OUT/$cfg.elf" "$OUT/hex/$cfg.hex" >> "$log" 2>&1 || ok=0
                set -- $($SIZE "$OUT/$cfg.elf" | awk 'NR == 2 { print $1, $2, $3 }')
                flim=$(ldsize "$ld" FLASH)
                rlim=$(ldsize "$ld" RAM)
                line="flash $(pct $(( $1 + $2 )) "$flim")  ram $(pct $(( $2 + $3 )) "$rlim")"
            else
                ok=0
            fi
        fi

        # binutils 2.39 and later remark that the single LOAD segment of a bare
        # metal image is both writable and executable.  That is what these
        # linker scripts describe on purpose, so it is not counted.
        warn=$(grep "warning:" "$log" | grep -vc "LOAD segment with RWX" || true)
        st=OK
        if [ "$ok" -ne 1 ]; then
            st=FAIL
            rc=1
            failed=$(( failed + 1 ))
        elif [ "$STRICT" = 1 ] && [ "$warn" -gt 0 ]; then
            rc=1
        fi
        built=$(( built + 1 ))

        printf '%-10s %-5s warn %-4s %s\n' "$cfg" "$st" "$warn" "$line" | tee -a "$OUT/summary.txt"
        rm -f "$one"
    done
done

printf -- '---\n%s configuration(s), %s failed\n' "$built" "$failed" | tee -a "$OUT/summary.txt"
exit $rc
