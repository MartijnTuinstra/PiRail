<?php

ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ERROR | E_WARNING | E_PARSE);

$Signal_types = array(
'<path d="M 15,5 v 10 a 5,5 0,0,1 -10,0 v -10 a 5,5 0,0,1 10,0" style="stroke-width:0px;stroke:black;fill:black;"/><circle cx="10" cy=" 5" r="2" style="stroke-width:0px;fill:red"/><circle cx="10" cy="10" r="2" style="stroke-width:0px;fill:orange"/><circle cx="10" cy="15" r="2" style="stroke-width:0px;fill:lime"/>',
'<path d="M 15,5 v 5 a 5,5 0,0,1 -10,0 v -5 a 5,5 0,0,1 10,0" style="stroke-width:0px;stroke:black;fill:black;"/><circle cx="10" cy=" 5" r="2" style="stroke-width:0px;fill:red"/><circle cx="10" cy="10" r="2" style="stroke-width:0px;fill:green"/>');

$part_content = '{
    "Rail": [
        {
            "nr": 0,
            "part": "RS",
            "transform": "translate(361,288) rotate(0)",
            "BlockID": 0,
            "length": 3,
            "x": "82",
            "y": "0"
        },
        {
            "nr": 1,
            "part": "RS",
            "transform": "translate(361,317.2893) rotate(0)",
            "BlockID": 11,
            "length": 3,
            "x": "82",
            "y": "0"
        },
        {
            "nr": 2,
            "part": "RS",
            "transform": "translate(443,288) rotate(0)",
            "BlockID": 1,
            "length": 1,
            "x": "35.36",
            "y": "0"
        },
        {
            "nr": 3,
            "part": "RS",
            "transform": "translate(478.36,288) rotate(0)",
            "BlockID": 1,
            "length": 1,
            "x": "35.36",
            "y": "0"
        },
        {
            "nr": 4,
            "part": "RS",
            "transform": "translate(513.72,288) rotate(0)",
            "BlockID": 1,
            "length": 1,
            "x": "35.36",
            "y": "0"
        },
        {
            "nr": 5,
            "part": "RS",
            "transform": "translate(549.08,288) rotate(0)",
            "BlockID": 1,
            "length": 1,
            "x": "35.36",
            "y": "0"
        },
        {
            "nr": 6,
            "part": "RS",
            "transform": "translate(584.44,288) rotate(0)",
            "BlockID": 6,
            "length": 1,
            "x": "35.36",
            "y": "0"
        },
        {
            "nr": 7,
            "part": "RS",
            "transform": "translate(443,317.2893) rotate(0)",
            "BlockID": 13,
            "length": 1,
            "x": "35.36",
            "y": "0"
        },
        {
            "nr": 8,
            "part": "RS",
            "transform": "translate(478.36,317.2893) rotate(0)",
            "BlockID": 13,
            "length": 1,
            "x": "35.36",
            "y": "0"
        },
        {
            "nr": 9,
            "part": "RS",
            "transform": "translate(513.72,317.2893) rotate(0)",
            "BlockID": 13,
            "length": 1,
            "x": "35.36",
            "y": "0"
        },
        {
            "nr": 10,
            "part": "RS",
            "transform": "translate(549.08,317.2893) rotate(0)",
            "BlockID": 13,
            "length": 1,
            "x": "35.36",
            "y": "0"
        },
        {
            "nr": 11,
            "part": "RS",
            "transform": "translate(584.44,317.2893) rotate(0)",
            "BlockID": 14,
            "length": 1,
            "x": "35.36",
            "y": "0"
        },
        {
            "nr": 12,
            "part": "RS",
            "transform": "translate(619.8000000000001,288) rotate(0)",
            "BlockID": 6,
            "length": 4,
            "x": "162",
            "y": "0"
        },
        {
            "nr": 13,
            "part": "RS",
            "transform": "translate(619.8000000000001,317.2893) rotate(0)",
            "BlockID": 14,
            "length": 4,
            "x": "162",
            "y": "0"
        },
        {
            "nr": 14,
            "part": "RS",
            "transform": "translate(781.8000000000001,288) rotate(0)",
            "BlockID": 7,
            "length": 4,
            "x": "162",
            "y": "0"
        },
        {
            "nr": 15,
            "part": "RS",
            "transform": "translate(781.8000000000001,317.2893) rotate(0)",
            "BlockID": 15,
            "length": 4,
            "x": "162",
            "y": "0"
        },
        {
            "nr": 16,
            "part": "RS",
            "transform": "translate(943.8000000000001,288) rotate(0)",
            "BlockID": 8,
            "length": 4,
            "x": "162",
            "y": "0"
        },
        {
            "nr": 17,
            "part": "RS",
            "transform": "translate(943.8000000000001,317.2893) rotate(0)",
            "BlockID": 16,
            "length": 4,
            "x": "162",
            "y": "0"
        },
        {
            "nr": 18,
            "part": "RS",
            "transform": "translate(1105.8000000000002,288) rotate(0)",
            "BlockID": 9,
            "length": 4,
            "x": "162",
            "y": "0"
        },
        {
            "nr": 19,
            "part": "RS",
            "transform": "translate(1105.8000000000002,317.2893) rotate(0)",
            "BlockID": 17,
            "length": 4,
            "x": "162",
            "y": "0"
        },
        {
            "nr": 20,
            "part": "RS",
            "transform": "translate(1267.8000000000002,288) rotate(0)",
            "BlockID": 9,
            "length": 1,
            "x": "35.36",
            "y": "0"
        },
        {
            "nr": 21,
            "part": "RS",
            "transform": "translate(1303.16,288) rotate(0)",
            "BlockID": 10,
            "length": 1,
            "x": "35.36",
            "y": "0"
        },
        {
            "nr": 22,
            "part": "RS",
            "transform": "translate(1338.52,288) rotate(0)",
            "BlockID": 10,
            "length": 1,
            "x": "35.36",
            "y": "0"
        },
        {
            "nr": 23,
            "part": "RS",
            "transform": "translate(1373.8799999999999,288) rotate(0)",
            "BlockID": 10,
            "length": 1,
            "x": "35.36",
            "y": "0"
        },
        {
            "nr": 24,
            "part": "RS",
            "transform": "translate(1409.2399999999998,288) rotate(0)",
            "BlockID": 10,
            "length": 1,
            "x": "35.36",
            "y": "0"
        },
        {
            "nr": 25,
            "part": "RS",
            "transform": "translate(1267.8000000000002,317.2893) rotate(0)",
            "BlockID": 17,
            "length": 1,
            "x": "35.36",
            "y": "0"
        },
        {
            "nr": 26,
            "part": "RS",
            "transform": "translate(1303.16,317.2893) rotate(0)",
            "BlockID": 26,
            "length": 1,
            "x": "35.36",
            "y": "0"
        },
        {
            "nr": 27,
            "part": "RS",
            "transform": "translate(1338.52,317.2893) rotate(0)",
            "BlockID": 26,
            "length": 1,
            "x": "35.36",
            "y": "0"
        },
        {
            "nr": 28,
            "part": "RS",
            "transform": "translate(1373.8799999999999,317.2893) rotate(0)",
            "BlockID": 26,
            "length": 1,
            "x": "35.36",
            "y": "0"
        },
        {
            "nr": 29,
            "part": "RS",
            "transform": "translate(1409.2399999999998,317.2893) rotate(0)",
            "BlockID": 26,
            "length": 1,
            "x": "35.36",
            "y": "0"
        },
        {
            "nr": 30,
            "part": "RS",
            "transform": "translate(1444.5999999999997,288) rotate(0)",
            "BlockID": 11,
            "length": 3,
            "x": "82",
            "y": "0"
        },
        {
            "nr": 31,
            "part": "RS",
            "transform": "translate(1444.5999999999997,317.2893) rotate(0)",
            "BlockID": 11,
            "length": 3,
            "x": "82",
            "y": "0"
        },
        {
            "nr": 32,
            "part": "RC",
            "transform": "translate(478.3646609406727,302.6446390593274) rotate(45)",
            "BlockID": 13,
            "angle": 3,
            "radius": 3,
            "d": "M 0,0 a 50,50 0,0,0 35.35533905932737,-14.64466094067262",
            "x": "35.35533905932737",
            "y": "-14.64466094067262"
        },
        {
            "nr": 33,
            "part": "RC",
            "transform": "translate(513.72,288) rotate(0)",
            "BlockID": 1,
            "angle": 3,
            "radius": 3,
            "d": "M 0,0 a 50,50 0,0,0 35.35533905932737,-14.64466094067262",
            "x": "35.35533905932737",
            "y": "-14.64466094067262"
        },
        {
            "nr": 34,
            "part": "RC",
            "transform": "translate(584.4306781186547,258.71067811865476) rotate(180)",
            "BlockID": 1,
            "angle": 3,
            "radius": 3,
            "d": "M 0,0 a 50,50 0,0,0 35.35533905932737,-14.64466094067262",
            "x": "35.35533905932737",
            "y": "-14.64466094067262"
        },
        {
            "nr": 35,
            "part": "RC",
            "transform": "translate(478.3553390593274,302.6446609406726) rotate(225)",
            "BlockID": 1,
            "angle": 3,
            "radius": 3,
            "d": "M 0,0 a 50,50 0,0,0 35.35533905932737,-14.64466094067262",
            "x": "35.35533905932737",
            "y": "-14.64466094067262"
        },
        {
            "nr": 36,
            "part": "RC",
            "transform": "translate(549.0753390593273,331.93396094067265) rotate(225)",
            "BlockID": 13,
            "angle": 3,
            "radius": 3,
            "d": "M 0,0 a 50,50 0,0,0 35.35533905932737,-14.64466094067262",
            "x": "35.35533905932737",
            "y": "-14.64466094067262"
        },
        {
            "nr": 37,
            "part": "RC",
            "transform": "translate(549.0753390593273,331.93396094067265) rotate(45)",
            "BlockID": 13,
            "angle": 3,
            "radius": 3,
            "d": "M 0,0 a 50,50 0,0,0 35.35533905932737,-14.64466094067262",
            "x": "35.35533905932737",
            "y": "-14.64466094067262"
        },
        {
            "nr": 38,
            "part": "RS",
            "transform": "translate(584.4306781186547,258.71067811865476) rotate(0)",
            "BlockID": 2,
            "length": 1,
            "x": "35.36",
            "y": "0"
        },
        {
            "nr": 39,
            "part": "RS",
            "transform": "translate(584.4306781186547,346.57862188134527) rotate(0)",
            "BlockID": 18,
            "length": 1,
            "x": "35.36",
            "y": "0"
        },
        {
            "nr": 40,
            "part": "RC",
            "transform": "translate(1373.8799999999999,317.2893) rotate(0)",
            "BlockID": 26,
            "angle": 3,
            "radius": 3,
            "d": "M 0,0 a 50,50 0,0,0 35.35533905932737,-14.64466094067262",
            "x": "35.35533905932737",
            "y": "-14.64466094067262"
        },
        {
            "nr": 41,
            "part": "RC",
            "transform": "translate(1373.8799999999999,317.2893) rotate(180)",
            "BlockID": 26,
            "angle": 3,
            "radius": 3,
            "d": "M 0,0 a 50,50 0,0,0 35.35533905932737,-14.64466094067262",
            "x": "35.35533905932737",
            "y": "-14.64466094067262"
        },
        {
            "nr": 42,
            "part": "RC",
            "transform": "translate(1338.5246609406724,273.3553390593274) rotate(45)",
            "BlockID": 10,
            "angle": 3,
            "radius": 3,
            "d": "M 0,0 a 50,50 0,0,0 35.35533905932737,-14.64466094067262",
            "x": "35.35533905932737",
            "y": "-14.64466094067262"
        },
        {
            "nr": 43,
            "part": "RC",
            "transform": "translate(1444.5906781186548,287.9999781186548) rotate(180)",
            "BlockID": 10,
            "angle": 3,
            "radius": 3,
            "d": "M 0,0 a 50,50 0,0,0 35.35533905932737,-14.64466094067262",
            "x": "35.35533905932737",
            "y": "-14.64466094067262"
        },
        {
            "nr": 44,
            "part": "RC",
            "transform": "translate(1303.169321881345,346.57862188134527) rotate(0)",
            "BlockID": 42,
            "angle": 3,
            "radius": 3,
            "d": "M 0,0 a 50,50 0,0,0 35.35533905932737,-14.64466094067262",
            "x": "35.35533905932737",
            "y": "-14.64466094067262"
        },
        {
            "nr": 45,
            "part": "RC",
            "transform": "translate(1338.5246609406724,273.3553390593274) rotate(225)",
            "BlockID": 10,
            "angle": 3,
            "radius": 3,
            "d": "M 0,0 a 50,50 0,0,0 35.35533905932737,-14.64466094067262",
            "x": "35.35533905932737",
            "y": "-14.64466094067262"
        },
        {
            "nr": 46,
            "part": "RS",
            "transform": "translate(1267.809321881345,258.71067811865476) rotate(0)",
            "BlockID": 5,
            "length": 1,
            "x": "35.36",
            "y": "0"
        },
        {
            "nr": 47,
            "part": "RS",
            "transform": "translate(1267.809321881345,346.57862188134527) rotate(0)",
            "BlockID": 21,
            "length": 1,
            "x": "35.36",
            "y": "0"
        },
        {
            "nr": 48,
            "part": "RS",
            "transform": "translate(619.7906781186547,258.71067811865476) rotate(0)",
            "BlockID": 2,
            "length": 4,
            "x": "162",
            "y": "0"
        },
        {
            "nr": 49,
            "part": "RS",
            "transform": "translate(781.7906781186547,258.71067811865476) rotate(0)",
            "BlockID": 3,
            "length": 4,
            "x": "162",
            "y": "0"
        },
        {
            "nr": 50,
            "part": "RS",
            "transform": "translate(943.7906781186547,258.71067811865476) rotate(0)",
            "BlockID": 4,
            "length": 4,
            "x": "162",
            "y": "0"
        },
        {
            "nr": 51,
            "part": "RS",
            "transform": "translate(1105.809321881345,258.71067811865476) rotate(0)",
            "BlockID": 5,
            "length": 4,
            "x": "162",
            "y": "0"
        },
        {
            "nr": 52,
            "part": "RS",
            "transform": "translate(619.7906781186547,346.57862188134527) rotate(0)",
            "BlockID": 18,
            "length": 4,
            "x": "162",
            "y": "0"
        },
        {
            "nr": 53,
            "part": "RS",
            "transform": "translate(781.7906781186547,346.57862188134527) rotate(0)",
            "BlockID": 19,
            "length": 4,
            "x": "162",
            "y": "0"
        },
        {
            "nr": 54,
            "part": "RS",
            "transform": "translate(943.7906781186547,346.57862188134527) rotate(0)",
            "BlockID": 20,
            "length": 4,
            "x": "162",
            "y": "0"
        },
        {
            "nr": 55,
            "part": "RS",
            "transform": "translate(1105.809321881345,346.57862188134527) rotate(0)",
            "BlockID": 21,
            "length": 4,
            "x": "162",
            "y": "0"
        },
        {
            "nr": 56,
            "part": "RS",
            "transform": "translate(549.0753390593273,331.93396094067265) rotate(45)",
            "BlockID": 13,
            "length": 2,
            "x": "42",
            "y": "0"
        },
        {
            "nr": 57,
            "part": "RS",
            "transform": "translate(1308.8261761308374,361.63244575050766) rotate(315)",
            "BlockID": 26,
            "length": 2,
            "x": "42",
            "y": "0"
        },
        {
            "nr": 58,
            "part": "RS",
            "transform": "translate(619.7813562373093,375.8679437626905) rotate(0)",
            "BlockID": 22,
            "length": 4,
            "x": "162",
            "y": "0"
        },
        {
            "nr": 59,
            "part": "RS",
            "transform": "translate(781.7813562373093,375.8679437626905) rotate(0)",
            "BlockID": 23,
            "length": 4,
            "x": "162",
            "y": "0"
        },
        {
            "nr": 60,
            "part": "RS",
            "transform": "translate(943.7813562373093,375.8679437626905) rotate(0)",
            "BlockID": 24,
            "length": 4,
            "x": "162",
            "y": "0"
        },
        {
            "nr": 61,
            "part": "RS",
            "transform": "translate(1105.7813562373094,375.8679437626905) rotate(0)",
            "BlockID": 25,
            "length": 4,
            "x": "162",
            "y": "0"
        },
        {
            "nr": 62,
            "part": "CRail",
            "transform": "translate(619.7813562373094,375.86794376269063) rotate(0)",
            "BlockID": 22,
            "d": "M 0,0 l -6.639999999995325,0 a 48.603030380332314,48.603030380332314 0,0,1 -34.367532368151615,-14.235498012182859",
            "r1": 0,
            "r2": -135,
            "x": -41.007532368147,
            "y": -14.235498012183
        },
        {
            "nr": 63,
            "part": "CRail",
            "transform": "translate(1267.7813562373094,375.86794376269063) rotate(0)",
            "BlockID": 25,
            "d": "M 0,0 l 6.677287525343672,0 a 48.60303038037068,48.60303038037068 0,0,0 34.36753236818436,-14.235498012182859",
            "r1": -180,
            "r2": 315,
            "x": 41.044819893528,
            "y": -14.235498012183
        }
    ],
    "Nodes": [
        {
            "nr": 0,
            "part": "MAN",
            "transform": "translate(361,288) rotate(0)",
            "length": 2
        },
        {
            "nr": 1,
            "part": "MAN",
            "transform": "translate(1526.5999999999997,317.2893) rotate(180)",
            "length": 2
        },
        {
            "nr": 2,
            "part": "SwN",
            "transform": "translate(443,288) rotate(-180)",
            "SwitchPID": 0,
            "Pstates": [
                {
                    "nr": "0",
                    "BID": 0
                }
            ],
            "Nstates": [
                {
                    "nr": "2",
                    "BID": 1
                },
                {
                    "nr": "35",
                    "BID": 1
                }
            ],
            "ConRailPart": "0",
            "Pstate": 1,
            "Nstate": 2
        },
        {
            "nr": 3,
            "part": "SwN",
            "transform": "translate(513.72,317.2893) rotate(-180)",
            "SwitchPID": 3,
            "SwitchNID": 2,
            "Pstates": [
                {
                    "nr": "8",
                    "BID": 13
                },
                {
                    "nr": "32",
                    "BID": 13
                }
            ],
            "Nstates": [
                {
                    "nr": "36",
                    "BID": 13
                },
                {
                    "nr": "9",
                    "BID": 13
                }
            ],
            "ConRailPart": "8",
            "Pstate": 2,
            "Nstate": 2
        },
        {
            "nr": 4,
            "part": "SwN",
            "transform": "translate(513.72,288) rotate(-180)",
            "SwitchPID": 1,
            "Pstates": [
                {
                    "nr": "3",
                    "BID": 1
                }
            ],
            "Nstates": [
                {
                    "nr": "4",
                    "BID": 1
                },
                {
                    "nr": "33",
                    "BID": 1
                }
            ],
            "ConRailPart": "3",
            "Pstate": 1,
            "Nstate": 2
        },
        {
            "nr": 5,
            "part": "SwN",
            "transform": "translate(1444.5999999999997,288) rotate(0)",
            "SwitchNID": 5,
            "Pstates": [
                {
                    "nr": "24",
                    "BID": 10
                },
                {
                    "nr": "43",
                    "BID": 10
                }
            ],
            "Nstates": [
                {
                    "nr": "30",
                    "BID": 11
                }
            ],
            "ConRailPart": "24",
            "Pstate": 1,
            "Nstate": 2
        },
        {
            "nr": 6,
            "part": "SwN",
            "transform": "translate(1373.8799999999999,288) rotate(0)",
            "SwitchNID": 6,
            "Pstates": [
                {
                    "nr": "22",
                    "BID": 10
                },
                {
                    "nr": "42",
                    "BID": 10
                }
            ],
            "Nstates": [
                {
                    "nr": "23",
                    "BID": 10
                }
            ],
            "ConRailPart": "22",
            "Pstate": 1,
            "Nstate": 2
        },
        {
            "nr": 7,
            "part": "SwN",
            "transform": "translate(1373.8799999999999,317.2893) rotate(0)",
            "SwitchPID": 7,
            "SwitchNID": 8,
            "Pstates": [
                {
                    "nr": "27",
                    "BID": 26
                },
                {
                    "nr": "41",
                    "BID": 26
                }
            ],
            "Nstates": [
                {
                    "nr": "28",
                    "BID": 26
                },
                {
                    "nr": "40",
                    "BID": 26
                }
            ],
            "ConRailPart": "27",
            "Pstate": 2,
            "Nstate": 2
        },
        {
            "nr": 8,
            "part": "SwN",
            "transform": "translate(549.0753390593273,331.93396094067265) rotate(-135)",
            "SwitchPID": 4,
            "Pstates": [
                {
                    "nr": "36",
                    "BID": 13
                }
            ],
            "Nstates": [
                {
                    "nr": "37",
                    "BID": 13
                },
                {
                    "nr": "56",
                    "BID": 13
                }
            ],
            "ConRailPart": "36",
            "Pstate": 1,
            "Nstate": 2
        },
        {
            "nr": 9,
            "part": "SwN",
            "transform": "translate(1338.5246609406724,331.93396094067265) rotate(-45)",
            "SwitchPID": 9,
            "Pstates": [
                {
                    "nr": "41",
                    "BID": 26
                }
            ],
            "Nstates": [
                {
                    "nr": "44",
                    "BID": 42
                },
                {
                    "nr": "57",
                    "BID": 26
                }
            ],
            "ConRailPart": "41",
            "Pstate": 1,
            "Nstate": 2
        },
        {
            "nr": 10,
            "part": "BI",
            "transform": "translate(443,288) rotate(0)",
            "AnchorID": 4,
            "ConRailPart": "0"
        },
        {
            "nr": 11,
            "part": "BI",
            "transform": "translate(443,317.2893) rotate(0)",
            "AnchorID": 17,
            "ConRailPart": "1"
        },
        {
            "nr": 12,
            "part": "BI",
            "transform": "translate(584.44,317.2893) rotate(0)",
            "AnchorID": 22,
            "ConRailPart": "10"
        },
        {
            "nr": 13,
            "part": "BI",
            "transform": "translate(584.4306781186547,346.57862188134527) rotate(0)",
            "AnchorID": 39,
            "ConRailPart": "37"
        },
        {
            "nr": 14,
            "part": "BI",
            "transform": "translate(578.7738238691624,361.63244575050766) rotate(45)",
            "AnchorID": 54,
            "ConRailPart": "56"
        },
        {
            "nr": 15,
            "part": "BI",
            "transform": "translate(584.44,288) rotate(0)",
            "AnchorID": 19,
            "ConRailPart": "5"
        },
        {
            "nr": 16,
            "part": "BI",
            "transform": "translate(584.4306781186547,258.71067811865476) rotate(0)",
            "AnchorID": 38,
            "ConRailPart": "34"
        },
        {
            "nr": 17,
            "part": "BI",
            "transform": "translate(1308.8261761308374,361.63244575050766) rotate(-45)",
            "AnchorID": 55,
            "ConRailPart": "57"
        },
        {
            "nr": 18,
            "part": "BI",
            "transform": "translate(1303.169321881345,346.57862188134527) rotate(0)",
            "AnchorID": 44,
            "ConRailPart": "44"
        },
        {
            "nr": 19,
            "part": "BI",
            "transform": "translate(1303.16,317.2893) rotate(0)",
            "AnchorID": 33,
            "ConRailPart": "25"
        },
        {
            "nr": 20,
            "part": "BI",
            "transform": "translate(1303.16,288) rotate(0)",
            "AnchorID": 32,
            "ConRailPart": "20"
        },
        {
            "nr": 21,
            "part": "BI",
            "transform": "translate(1303.169321881345,258.71067811865476) rotate(0)",
            "AnchorID": 45,
            "ConRailPart": "45"
        },
        {
            "nr": 22,
            "part": "BI",
            "transform": "translate(478.3646609406727,302.6446390593274) rotate(45)",
            "AnchorID": 36,
            "ConRailPart": "32"
        },
        {
            "nr": 23,
            "part": "BI",
            "transform": "translate(1409.2353390593273,302.6446390593274) rotate(-45)",
            "AnchorID": 42,
            "ConRailPart": "40"
        },
        {
            "nr": 24,
            "part": "BI",
            "transform": "translate(1444.5999999999997,317.2893) rotate(0)",
            "AnchorID": 35,
            "ConRailPart": "29"
        },
        {
            "nr": 25,
            "part": "BI",
            "transform": "translate(781.7906781186547,258.71067811865476) rotate(0)",
            "AnchorID": 48,
            "ConRailPart": "48"
        },
        {
            "nr": 26,
            "part": "BI",
            "transform": "translate(781.8000000000001,288) rotate(0)",
            "AnchorID": 24,
            "ConRailPart": "12"
        },
        {
            "nr": 27,
            "part": "BI",
            "transform": "translate(781.8000000000001,317.2893) rotate(0)",
            "AnchorID": 25,
            "ConRailPart": "13"
        },
        {
            "nr": 28,
            "part": "BI",
            "transform": "translate(781.7906781186547,346.57862188134527) rotate(0)",
            "AnchorID": 51,
            "ConRailPart": "52"
        },
        {
            "nr": 29,
            "part": "BI",
            "transform": "translate(781.7813562373093,375.8679437626905) rotate(0)",
            "AnchorID": 57,
            "ConRailPart": "58"
        },
        {
            "nr": 30,
            "part": "BI",
            "transform": "translate(943.7906781186547,258.71067811865476) rotate(0)",
            "AnchorID": 49,
            "ConRailPart": "49"
        },
        {
            "nr": 31,
            "part": "BI",
            "transform": "translate(943.8000000000001,288) rotate(0)",
            "AnchorID": 26,
            "ConRailPart": "14"
        },
        {
            "nr": 32,
            "part": "BI",
            "transform": "translate(943.8000000000001,317.2893) rotate(0)",
            "AnchorID": 27,
            "ConRailPart": "15"
        },
        {
            "nr": 33,
            "part": "BI",
            "transform": "translate(943.7906781186547,346.57862188134527) rotate(0)",
            "AnchorID": 52,
            "ConRailPart": "53"
        },
        {
            "nr": 34,
            "part": "BI",
            "transform": "translate(943.7813562373093,375.8679437626905) rotate(0)",
            "AnchorID": 58,
            "ConRailPart": "59"
        },
        {
            "nr": 35,
            "part": "BI",
            "transform": "translate(1105.7906781186548,258.71067811865476) rotate(0)",
            "AnchorID": 50,
            "ConRailPart": "50"
        },
        {
            "nr": 36,
            "part": "BI",
            "transform": "translate(1105.8000000000002,288) rotate(0)",
            "AnchorID": 28,
            "ConRailPart": "16"
        },
        {
            "nr": 37,
            "part": "BI",
            "transform": "translate(1105.8000000000002,317.2893) rotate(0)",
            "AnchorID": 29,
            "ConRailPart": "17"
        },
        {
            "nr": 38,
            "part": "BI",
            "transform": "translate(1105.7906781186548,346.57862188134527) rotate(0)",
            "AnchorID": 53,
            "ConRailPart": "54"
        },
        {
            "nr": 39,
            "part": "BI",
            "transform": "translate(1105.7813562373094,375.8679437626905) rotate(0)",
            "AnchorID": 59,
            "ConRailPart": "60"
        },
        {
            "nr": 40,
            "part": "BI",
            "transform": "translate(1444.5999999999997,288) rotate(0)",
            "AnchorID": 10,
            "ConRailPart": "24"
        }
    ],
    "Signals": [
        {
            "nr": 0,
            "transform": "translate(619.7813562373093,375.8679437626905) rotate(-90)",
            "setup": {
                "type": 0,
                "heading": 0
            }
        },
        {
            "nr": 1,
            "transform": "translate(619.7906781186547,346.57862188134527) rotate(-90)",
            "setup": {
                "type": 0,
                "heading": 0
            }
        },
        {
            "nr": 2,
            "transform": "translate(619.8000000000001,317.2893) rotate(-90)",
            "setup": {
                "type": 0,
                "heading": 0
            }
        },
        {
            "nr": 3,
            "transform": "translate(619.8000000000001,288) rotate(-90)",
            "setup": {
                "type": 0,
                "heading": 0
            }
        },
        {
            "nr": 4,
            "transform": "translate(619.7906781186547,258.71067811865476) rotate(-90)",
            "setup": {
                "type": 0,
                "heading": 0
            }
        },
        {
            "nr": 5,
            "transform": "translate(1267.7813562373094,375.8679437626905) rotate(-270)",
            "setup": {
                "type": 0,
                "heading": 0
            }
        },
        {
            "nr": 6,
            "transform": "translate(1267.809321881345,346.57862188134527) rotate(90)",
            "setup": {
                "type": 0,
                "heading": 0
            }
        },
        {
            "nr": 7,
            "transform": "translate(1267.8000000000002,317.2893) rotate(-270)",
            "setup": {
                "type": 0,
                "heading": 0
            }
        }
    ]
}';

$data = json_decode($part_content,true);

//var_dump($data);
$x_max = 0;
$x_min = 5000;
$y_max = 0;
$y_min = 5000;

function min_max($x,$y){
	global $x_min,$x_max,$y_min,$y_max;

	$x_max = max($x_max,$x);
	$y_max = max($y_max,$y);
	$x_min = min($x_min,$x);
	$y_min = min($y_min,$y);
}

//Find Boundaries
foreach ($data['Rail'] as $Rail){
	$var = explode(" ",$Rail['transform']);
	$rotation = intval(substr($var[1],7,-1));
	$x_y = explode(",",substr($var[0],10,-1));
	//First point
	min_max(floatval($x_y[0]),floatval($x_y[1]));
	//Second point
	$x_y2 = [];
	$x_y2[0] = $x_y[0] + $Rail['x'] * cos(deg2rad($rotation));
	$x_y2[1] = $x_y[1] + $Rail['x'] * sin(deg2rad($rotation));
	min_max(floatval($x_y2[0]),floatval($x_y2[1]));
}

echo "<h2>Min Max</h2><br/>";
echo "X max: ".$x_max;
echo "X min: ".$x_min;
echo "Y max: ".$y_max;
echo "Y min: ".$y_min."<br/>";

$Blocks = array();
$Switches = array();

//Shift all rail segments into blocks
foreach ($data['Rail'] as $Rail){
	$var = explode(" ",$Rail['transform']);
	$rotation = intval(substr($var[1],7,-1));
	$x_y = explode(",",substr($var[0],10,-1));

	$x_y[0] = floatval($x_y[0]) - $x_min;
	$x_y[1] = floatval($x_y[1]) - $y_min +10;

	$Rail['transform'] = "translate(".$x_y[0].",".$x_y[1].") rotate(".$rotation.")";

	if(!is_array ($Blocks[$Rail['BlockID']])){
		$Blocks[$Rail['BlockID']] = array();
	}
	if(!is_array ($Blocks[$Rail['BlockID']][$Rail['nr']])){
		$Blocks[$Rail['BlockID']][$Rail['nr']] = $Rail;
	}
}
echo "<script>console.log('Raw Switches');</script>";
echo "<script>console.log(".json_encode($data['Nodes']).");</script>";
//Shift switches in array
foreach ($data['Nodes'] as $Switch){
	if($Switch["part"] == "SwN"){
		if(isset($Switch['SwitchPID']) && count($Switch['Pstates']) > 1){
			if(!is_array ($Switches[$Switch['Pstates'][0]['BID']])){
				$Switches[$Switch['Pstates'][0]['BID']] = array();
			}
			$Switches[$Switch['Pstates'][0]['BID']][$Switch['SwitchPID']] = array("ID"=>$Switch['SwitchPID'],"States"=>$Switch['Pstates']);
		}
		if(isset($Switch['SwitchNID']) && count($Switch['Nstates']) > 1){
			if(!is_array ($Switches[$Switch['Nstates'][0]['BID']])){
				$Switches[$Switch['Nstates'][0]['BID']] = array();
			}
			$Switches[$Switch['Nstates'][0]['BID']][$Switch['SwitchNID']] = array("ID"=>$Switch['SwitchNID'],"States"=>$Switch['Nstates']);
		}
	}
}

echo "<script>console.log('Blocks');</script>";
echo "<script>console.log(".json_encode($Blocks).");</script>";
echo "<script>console.log('Switches');</script>";
echo "<script>console.log(".json_encode($Switches).");</script>";

echo "<svg width='".($x_max-$x_min)."' height='".($y_max-$y_min+30)."' viewBox='0 0 ".($x_max-$x_min)." ".($y_max-$y_min+30)."'>";

foreach($Blocks as $nr => $Block){

	echo '<g class="B'.$nr.'">';

	if(isset($Switches[$nr])){
		foreach($Switches[$nr] as $Switch){
			echo '<g class="Sw'.$Switch['ID'].'">';
			for($i = 0;$i<count($Switch['States']);$i++){
				//Display one, hide others
				$opacity = "";
				if($i > 0){
					$opacity = "opacity:0;";
				}
				//Other Rail
				foreach($Switch['States'] as $j => $SwStates){
					if($j == $i){
						continue;
					}
					$Rail = $Blocks[$nr][$SwStates['nr']];
					$Block[$SwStates['nr']]['Done'] = "Set";
					
					if($Rail['part'] == "CRail" || $Rail['part'] == "RC"){
						echo '<path class="L SwS'.$i.'o" nr="'.$Rail['nr'].'" part="CRail" transform="'.$Rail['transform'].'" d="'.$Rail['d'].'" style="fill:none;stroke:lightgrey;stroke-width:6px;'.$opacity.'"/>';
					}elseif($Rail['part'] == "RS"){
						echo '<line class="L SwS'.$i.'o" nr="'.$Rail['nr'].'" part="RS" transform="'.$Rail['transform'].'" x2="'.$Rail['x'].'" y2="'.$Rail['y'].'" style="fill:none;stroke:lightgrey;stroke-width:6px;'.$opacity.'"/>';
					}
				}

				//Selected Rail
				$Rail = $Blocks[$nr][$Switch['States'][$i]['nr']];
				$Block[$SwStates['nr']]['Done'] = "Set";
				
				if($Rail['part'] == "CRail" || $Rail['part'] == "RC"){
					echo '<path class="L SwS'.$i.'" nr="'.$Rail['nr'].'" part="CRail" transform="'.$Rail['transform'].'" d="'.$Rail['d'].'" style="fill:none;stroke:#555;stroke-width:6px;'.$opacity.'"/>';
				}elseif($Rail['part'] == "RS"){
					echo '<line class="L SwS'.$i.'" nr="'.$Rail['nr'].'" part="RS" transform="'.$Rail['transform'].'" x2="'.$Rail['x'].'" y2="'.$Rail['y'].'" style="fill:none;stroke:#555;stroke-width:6px;'.$opacity.'"/>';
				}

			}

			echo '</g>';
		}
	}

	foreach($Block as $Rail){
		if(isset($Rail['Done'])){
			echo '';
			continue;
		}else{
			if($Rail['part'] == "CRail" || $Rail['part'] == "RC"){
				echo '<path class="L" nr="'.$Rail['nr'].'" part="CRail" transform="'.$Rail['transform'].'" d="'.$Rail['d'].'" style="fill:none;stroke:#555;stroke-width:6px;"/>';
			}elseif($Rail['part'] == "RS"){
				echo '<line class="L" nr="'.$Rail['nr'].'" part="RS" transform="'.$Rail['transform'].'" x2="'.$Rail['x'].'" y2="'.$Rail['y'].'" style="fill:none;stroke:#555;stroke-width:6px;"/>';
			}
		}
	}

	echo '</g>';

}

echo "<g class='Signals'>";
foreach($data['Signals'] as $Signal){
	$var = explode(" ",$Signal['transform']);
	$rotation = intval(substr($var[1],7,-1));
	$x_y = explode(",",substr($var[0],10,-1));

	$x_y[0] = floatval($x_y[0]) - $x_min;
	$x_y[1] = floatval($x_y[1]) - $y_min +10;

	$Signal['transform'] = "translate(".$x_y[0].",".$x_y[1].") rotate(".$rotation.")";

	echo '<g transform="'.$Signal['transform'].'" class="Sig'.$Signal['nr'].'">';
	echo $Signal_types[$Signal['setup']['type']];
	echo '</g>';
}
echo "</g>";
    
echo "</svg>";
?>