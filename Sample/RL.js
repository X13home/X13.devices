// Running Lights
// Tmin 41 uS, Tmax 216 uS

var Op0, Op1, Op2, Op3, Op4, Op5;
var oldS = new UInt16(0), stat = new UInt8(1);

let sc = getMilliseconds() & 0xFFC0;

if(oldS != sc)
{
	oldS = sc;
	let s = stat << 1;
	if(s > 32)
		s = 1;
	stat = s;

	Op0 = ((s & 1) != 0);
	Op1 = ((s & 2) != 0);
	Op2 = ((s & 4) != 0);
	Op3 = ((s & 8) != 0);
	Op4 = ((s & 16) != 0);
	Op5 = ((s & 32) != 0);
}


