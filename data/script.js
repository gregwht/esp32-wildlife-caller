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
		.then(response => response.text())
		.then(message => {
			
			const saveMessage = document.getElementById("saveMessage");
			saveMessage.innerHTML = "Settings saved!";
			
			setTimeout(() => {
				saveMessage.innerHTML = "";
			}, 3000);

			loadSettings();
			
		});
}

function loadSettings() {

	fetch("/settings")
		.then(response => response.json())
		.then(settings => {

			document.getElementById("startTime01").value = settings.time01;

			let hours = Math.floor(settings.duration01 / 60);
			let minutes = settings.duration01 % 60;

			document.getElementById("duration01Hours").value = hours;
			document.getElementById("duration01Mins").value = minutes;

			document.getElementById("statusStartTime").innerHTML = settings.time01;

			document.getElementById("statusDuration").innerHTML = hours + "h " + minutes + "m";
		});
}

function loadStatus() {

	fetch("/status")
	.then(response => response.json())
	.then(status => {

		document.getElementById("currentTime").innerHTML = status.time;
	});
}

function syncRTC() {

	const now = new Date();

	const year = now.getFullYear();
	const month = now.getMonth() + 1;
	const weekday = now.getDay() === 0 ? 7 : now.getDay(); // JavaScript reports Sunday as 0, but the RTC library we're using reports it as 7, so we need to convert.
	const day = now.getDate();
	const hour = now.getHours();
	const minute = now.getMinutes();
	const second = now.getSeconds();

	fetch(`/setRTC?year=${year}&month=${month}&weekday=${weekday}&day=${day}&hour=${hour}&minute=${minute}&second=${second}`)
		.then(response => response.text())
		.then(message => {
			
			const rtcMessage = document.getElementById("rtcMessage");
			rtcMessage.innerHTML = "RTC Clock Synchronised!";
			
			setTimeout(() => {
				rtcMessage.innerHTML = "";
			}, 3000);

			loadStatus();
			
		});
}



window.onload = function() {

	loadSettings();
};

// Update every second so clock readout is accurate
setInterval(loadStatus, 1000);