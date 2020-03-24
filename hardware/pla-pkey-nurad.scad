w=8;        //key width, default 8
wc=10;      //mounting end outer diameter, default 10
l=15;       //key length (range normally increase in steps of 5)
hght=3;     //key height, default 3,3.5 and 4
angl=25;    //key end angle, default 25
estp=1.5;    //end step before angle, default 0.5
noext=0;    //set to 1 to get just the round part   

// 20/3,26/3.5,32/4 sk: 15/3 (estp 1.5)

 module prism(l, w, h){
       polyhedron(
               points=[[0,0,0], [l,0,0], [l,w,0], [0,w,0], [0,w,h], [l,w,h]],
               faces=[[0,1,2,3],[5,4,3,2],[0,4,5,1],[0,3,4],[5,2,1]]
               );
   }    
  
difference(){
    translate([0,0,0]) cylinder(h=hght,r=wc/2,,$fn=100);
    translate([0,0,hght-2]) cylinder(h=2.2,r1=1.6,r2=3.2,$fn=100);
    translate([0,0,-1]) cylinder(h=hght+5,r=1.6,$fn=100);
    //translate([-8,-8,0]) cube([16,2,3]);
    
}
if (!noext){
difference(){
    translate([-w/2,-l+1,0]) cube([w,l,hght]);
    translate([0,0,hght-2]) cylinder(h=2.2,r1=1.6,r2=3.2,$fn=100);
    translate([0,0,-1]) cylinder(h=hght+5,r=1.6,$fn=100); 
    translate([w/2-1,-l,hght]) rotate([180,0,90]) prism(l,1,1);  
    translate([-w/2-0.001,-l,hght-1]) rotate([90,0,90]) prism(l,1,1); 
    //translate([-w/2,-l+5.5,hght]) rotate([160,0,0])prism(w,15,15);
    translate([-w/2,-l,estp]) rotate([angl,0,0])cube(40,40,40);
}
}
