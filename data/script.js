function saveSettings() {

	const time01 = document.getElementById("startTime01").value;
	
	const hours01 = Number(document.getElementById("duration01Hours").value);
	const minutes01 = Number(document.getElementById("duration01Mins").value);

	if (hours01 === 24 && minutes01 > 0) {
		alert("The maximum duration is 24 hours.");
		return;
	}

	const duration01 = hours01 * 60 + minutes01;

	console.log(time01);
	console.log(duration01);

	fetch(`/save?time01=${time01}&duration01=${duration01}`)
}

window.onload = function() {

	fetch("/settings")
		.then(response => response.json())
		.then(settings => {

			document.getElementById("startTime01").value = settings.time01;

			let hours = Math.floor(settings.duration01 / 60);
			let minutes = settings.duration01 % 60;

			document.getElementById("duration01Hours").value = hours;
			document.getElementById("duration01Mins").value = minutes;
		});
};