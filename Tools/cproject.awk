# Extract the build settings of every configuration from an Eclipse CDT .cproject.
#
# Output, one tab-separated record per setting:
#   <configuration>  S|L  <superClass>  <value>
# S - a plain <option value="...">, L - one <listOptionValue> of a list option.
#
# Used by build-stm32.sh so that a command line build uses exactly the flags
# Eclipse would use, with no second copy of them to keep in sync.

function attr(line, nm,   r) {
  r = line
  if (match(r, "[ \t]" nm "=\"[^\"]*\"")) {
    r = substr(r, RSTART, RLENGTH)
    sub("^[ \t]" nm "=\"", "", r); sub("\"$", "", r)
    return r
  }
  return ""
}

# Only <configuration> carries artifactName; <cconfiguration> and the
# storageModule copies of the name do not.
/<configuration / && /artifactName=/ { cfg = attr($0, "name"); curopt = ""; next }
cfg == "" { next }
/<option / {
  sc = attr($0, "superClass"); v = attr($0, "value")
  if (v != "") { print cfg "\tS\t" sc "\t" v }
  # An option that is not self-closing holds <listOptionValue> children.
  if ($0 !~ /\/>[ \t]*$/) curopt = sc; else curopt = ""
  next
}
/<listOptionValue / { if (curopt != "") print cfg "\tL\t" curopt "\t" attr($0, "value"); next }
/<\/option>/ { curopt = "" ; next }
