{
name: "GEO",
index: "calibTube",
enable: 0,
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "detector",
type: "tube",
r_min: 0.0,
r_max: 11.1,
size_z: 635.0,
position: [1314.5, 0.0,825.5],
material: "steel",
color: [0.0, 1.0, 1.0, 0.1],
drawstyle: "solid"
}

{
name: "GEO",
index: "calibAir",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "calibTube",
type: "tube",
r_min: 0.0,
r_max: 10.1,
size_z: 635.0,
position: [0.0,0.0,0.0],
material: "steel",
color: [0.0, 1.0, 1.0, 0.1],
drawstyle: "solid"
}

