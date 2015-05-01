{
name: "GEO",
valid_begin: [0, 0],
valid_end: [0, 0],
gdml_file: "kpipe.gdml",
opdet_lv_name: "volActiveSiPM",
}

{
name: "GEO",
index: "border_target_inner",
valid_begin: [0, 0],
valid_end: [0,0],
mother:"border",
type:"border",
volume1:"pvScint",
volume2:"SiPM0",
//surface:"stainless_steel",
surface:"chip_silicon",
reverse: 1,
}