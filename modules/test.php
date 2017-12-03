<html>
	<head>
		<title>TEST</title>
		<script src="./../jquery-3.1.1.min.js"></script>
		<style>
			svg {
				shape-rendering:crispEdges;
			}
			.B {
				fill:none;
				stroke:#000000;
				stroke-width:10px;
				stroke-linecap:butt;
				stroke-linejoin:round;
				stroke-dasharray:none
			}
			.L {
				fill:none;
				stroke-width:4px;
				stroke-linecap:butt;
				stroke-linejoin:round;
				stroke-dasharray:none
			}
			.s {
				stroke:#000;
				fill:none;
				stroke-width:4px;
				stroke-linecap:round;
				stroke-linejoin:round;
				stroke-dasharray:none
			}
			* {
				margin: 0px;
				padding: 0px;
			}
			body,html {
				width:100%;
				height:100%;
			}
	    </style>
	</head>
	<body>
		<div id="M<?php echo($_GET['L']); ?>">
			<?php include("./".$_GET['L'].".svg"); ?>
		</div>
		<script src="./../script_0.8.js">
		</script>
		<button onClick="stop()">Stop</button>
		<button onClick="Start()">Start</button>
		<button onClick="update()">Once</button>
	</body>
</html>
