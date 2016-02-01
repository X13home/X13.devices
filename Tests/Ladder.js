var Ip0, Pp1, Pp2, Pp3, Pp4, Pp6, Pp7, Pp16, Pp17;
var lvl;

function Ladder()
{
    let i = lvl;
    if(Ip0) { if(i<400) i++; } else {if(i>1) i--; }
    
    lvl = i;

    if(i >= 1)  { Pp1 = i*i-1; } else return;
    if(i >= 17) { i-=16; Pp2 = i*i-1; } else return;
    if(i >= 17) { i-=16; Pp3 = i*i-1; } else return;
    if(i >= 17) { i-=16; Pp4 = i*i-1; } else return;
    if(i >= 17) { i-=16; Pp6 = i*i-1; } else return;
    if(i >= 17) { i-=16; Pp7 = i*i-1; } else return;
    if(i >= 17) { i-=16; Pp16 = i*i-1; } else return;
    if(i >= 17) { i-=16; Pp17 = i*i-1; }
}

Ladder();
