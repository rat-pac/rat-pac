// RATDB file describing simple detector geometry

// -------- GEO[world]
{
name: "GEO",
index: "world",
valid_begin: [0, 0],
valid_end: [0, 0],

mother: "", // world volume has no mother

type: "box",
size: [20000.0, 20000.0, 20000.0], // mm

material: "Rock",
color: [0.67, 0.29, 0.0],
}

// -------- GEO[cave]
{
name: "GEO",
index: "cave",
valid_begin: [0, 0],
valid_end: [0, 0],

mother: "world",

type: "box",
size: [7000.0, 7000.0, 7000.0], // mm

material: "Air",
color: [1.0, 1.0, 1.0],

position: [0.0, 0.0, 1950.0], // mm
}

// -------- GEO[tank]
{
name: "GEO",
index: "tank",
valid_begin: [0, 0],
valid_end: [0, 0],

mother: "cave",

type: "sphere",
r_max: 3600.0, // Outer radius, mm

material: "StainlessSteel",
color: [1.0, 1.0, 1.0],

position: [0.0, 0.0, 1950.0], // mm
}

// -------- GEO[buffer]
{
name: "GEO",
index: "buffer",
valid_begin: [0, 0],
valid_end: [0, 0],

mother: "tank",

type: "sphere",
r_max: 3500.0, // Outer radius, mm

material: "MineralOil",
color: [0.7, 0.4, 0.5, 0.1],
}

// -------- GEO[av] (acrylic vessel)
{
name: "GEO",
index: "av",
valid_begin: [0, 0],
valid_end: [0, 0],

mother: "buffer",

type: "sphere",
r_max: 2700.0, // Outer radius, mm

material: "Acrylic",
color: [1.0, 1.0, 0.0],
}

// -------- GEO[scint]
{
name: "GEO",
index: "scint",
valid_begin: [0, 0],
valid_end: [0, 0],

mother: "av",

type: "sphere",
r_max: 2600.0, // Outer radius, mm

material: "GdLoadedScint",
color: [0.6, 0.3, 1.0, 0.2],
}
