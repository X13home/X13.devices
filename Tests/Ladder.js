var Ip0, Pp2, Pp3, Pp4, Pp5, Pn6, Pn7, Pn8, Pn9, On10;
var lvl;

function Ladder()
{
    let i = lvl;
    if(Ip0) { if(i<420) i++; } else {if(i>1) i--; }
    
    lvl = i;

    if(i >= 1)  { Pp2 = i*i-1; } else return;
    if(i >= 25) { i-=24; Pp3 = i*i-1; } else return;
    if(i >= 25) { i-=24; Pp4 = i*i-1; } else return;
    if(i >= 25) { i-=24; Pp5 = i*i-1; } else return;
    if(i >= 25) { i-=24; Pn6 = i*i-1; } else return;
    if(i >= 25) { i-=24; Pn7 = i*i-1; } else return;
    if(i >= 25) { i-=24; Pn8 = i*i-1; } else return;
    if(i >= 25) { i-=24; Pn9 = i*i-1; } else return;
    On10 = (i > 200);
}

Ladder();
