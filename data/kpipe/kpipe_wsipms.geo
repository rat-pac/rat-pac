
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
volume1:"pvTarget",
volume2:"pvInnerPipe",
surface:"stainless_steel",
reverse: 1,
}

{
name: "GEO",
index: "border_veto_inner",
valid_begin: [0, 0],
valid_end: [0,0],
mother:"border",
type:"border",
volume1:"pvVetoRegion",
volume2:"pvInnerPipe",
surface:"stainless_steel",
reverse: 1,
}

{
name: "GEO",
index: "border_veto_outer",
valid_begin: [0, 0],
valid_end: [0,0],
mother:"border",
type:"border",
volume1:"pvVetoRegion",
volume2:"pvOuterPipe",
surface:"stainless_steel",
reverse: 1,
}


