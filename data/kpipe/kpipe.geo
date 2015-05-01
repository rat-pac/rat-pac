{
name: "GEO",
index: "world",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "", // world volume has no mother
type: "box",
size: [10000.0, 10000.0, 100000.0], // mm, half-length
material: "air",
invisible: 1,
}

{
name: "GEO",
index: "ground",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "world",
type: "box",
size: [3000.0, 3000.0, 100000.0], // mm, half-length
material: "dirt",
color: [0.67,0.165,0.165,0.1], 
position:[0000.0,-4920.0,0.0],
}

{
name: "GEO",
index: "outerpipe",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "world",
type: "tube",
r_max: 1920.0, // mm = 1.92 m, 0.5 m veto region
size_z: 50520.0, //mm = 100.52 m
position: [0.0, 0.0, 0.0],
material: "steel",
color: [1.0, 1.0, 1.0, 0.1],
}

{
name: "GEO",
index: "vetoregion",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "outerpipe",
type: "tube",
r_max: 1900.0, //mm = 1.4 m
size_z: 50500.0, //mm = 100 m
position: [0.0, 0.0, 0.0],
material: "scintillator",
color: [0.1, 0.0, 0.9, 1.0],
}

{
name: "GEO",
index: "innerpipe",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "outerpipe",
type: "tube",
r_max: 1420.0, //mm
size_z: 50020.0, //mm = 100 m
position: [0.0, 0.0, 0.0],
material: "steel",
color: [1.0, 1.0, 1.0, 1.0],
}

{
name: "GEO",
index: "targetvolume",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "innerpipe",
type: "tube",
r_max: 1400.0, //mm
size_z: 50000.0, //mm = 100 m
position: [0.0, 0.0, 0.0],
material: "scintillator",
color: [0.0, 0.0, 1.0, 1.0],
}