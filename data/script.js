const MAX_SLOTS = 10;

function addTimeslot(time = "09:00", durationMinutes = 60) {

	const container = document.getElementById("timeslotsContainer");

	if (container.children.length >= MAX_SLOTS) {
		alert(`You can only have up to ${MAX_SLOTS} timeslots.`);
		return;
	}

	const hours = Math.floor(durationMinutes / 60);
	const minutes = durationMinutes % 60;

	const row = document.createElement("div");
	row.className = "timeslot-row";

	row.innerHTML = `
		<label>
			Start Time
			<input type="time" class="slot-time" value="${time}">
		</label>

		<div class="duration-inputs">
			<label>
				Hours
				<input type="number" class="slot-hours" min="0" max="24" value="${hours}">
			</label>

			<label>
				Minutes
				<input type="number" class="slot-minutes" min="0" max="59" value="${minutes}">
			</label>
		</div>

		<button type="button" class="remove-btn" onclick="this.parentElement.remove()">Remove</button>
	`;

	container.appendChild(row);
}


function saveSettings() {

	const rows = document.querySelectorAll("#timeslotsContainer .timeslot-row");

	if (rows.length === 0) {
		alert("Add at least one timeslot before saving.");
		return;
	}

	let params = `numSlots=${rows.length}`;

	for (let i = 0; i < rows.length; i++) {

		const time = rows[i].querySelector(".slot-time").value;
		const hours = Number(rows[i].querySelector(".slot-hours").value);
		const minutes = Number(rows[i].querySelector(".slot-minutes").value);
	

		if (hours01 === 24 && minutes01 > 0) {
			alert(`Timeslot ${i + 1}: the maximum duration is 24 hours.`);
			return;
		}

		const duration = hours * 60 + minutes;

		console.log(`Slot ${i}:`, time, duration);

		params += `&time${i}=${time}&duration${i}=${duration}`;
	}

	fetch(`/save?${params}`)
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

			const container = document.getElementById("timeslotsContainer");
			container.innerHTML = ""; // Clear existing rows before rebuilding from the server's data

			settings.timeslots.forEach(slot => {
				addTimeslot(slot.time, slot.duration);
			});

			renderStatus(settings.timeslots);

		});
}

function renderStatus(timeslots) {

	const list = document.getElementById("statusTimeslots");
	list.innerHTML = "";

	if (timeslots.length === 0) {
		list.innerHTML = "<li>No timeslots configured</li>";
		return;
	}

	timeslots.forEach(slot => {

		const hours = Math.floor(slot.duration / 60);
		const minutes = slot.duration % 60;

		const item = document.createElement("li")
		item.textContent = `${slot.time} for ${hours}h ${minutes}m`;

		list.appendChild(item);
	})
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