const sosForm = document.getElementById("sosForm");
const getLocationBtn = document.getElementById("getLocation");
const locationInput = document.getElementById("location");
const latitudeInput = document.getElementById("latitude");
const longitudeInput = document.getElementById("longitude");
const locationStatus = document.getElementById("locationStatus");
const historyList = document.getElementById("historyList");
const requestCount = document.getElementById("requestCount");
const confirmation = document.getElementById("confirmation");
const confirmationText = document.getElementById("confirmationText");
const mapLink = document.getElementById("mapLink");

let requests = [];
let nextSOSId = 1001;

getLocationBtn.addEventListener("click", function () {
  if (!navigator.geolocation) {
    locationStatus.textContent =
      "Geolocation is not supported by this browser.";
    return;
  }

  locationStatus.textContent = "Getting your location...";
  getLocationBtn.disabled = true;

  navigator.geolocation.getCurrentPosition(
    function (position) {
      const lat = position.coords.latitude;
      const lon = position.coords.longitude;

      latitudeInput.value = lat.toFixed(6);
      longitudeInput.value = lon.toFixed(6);
      locationInput.value = `${lat.toFixed(6)}, ${lon.toFixed(6)}`;

      locationStatus.textContent =
        "Location obtained successfully.";
      getLocationBtn.disabled = false;
    },

    function (error) {
      const messages = {
        1: "Location permission denied. Allow access or enter your location manually.",
        2: "Location unavailable. Try again or enter your location manually.",
        3: "Location request timed out. Please try again."
      };

      locationStatus.textContent =
        messages[error.code] || "Could not get your location.";
      getLocationBtn.disabled = false;
    },

    {
      enableHighAccuracy: true,
      timeout: 15000,
      maximumAge: 0
    }
  );
});

function getValidCoordinates() {
  const latText = latitudeInput.value.trim();
  const lonText = longitudeInput.value.trim();

  if (latText === "" && lonText === "") {
    return null;
  }

  if (latText === "" || lonText === "") {
    return false;
  }

  const lat = Number(latText);
  const lon = Number(lonText);

  if (
    !Number.isFinite(lat) ||
    !Number.isFinite(lon) ||
    lat < -90 || lat > 90 ||
    lon < -180 || lon > 180
  ) {
    return false;
  }

  return { lat, lon };
}

function renderHistory() {
  requestCount.textContent = requests.length;

  historyList.replaceChildren();

  if (requests.length === 0) {
    const empty = document.createElement("p");
    empty.className = "empty";
    empty.textContent = "No requests submitted in this session.";
    historyList.appendChild(empty);
    return;
  }

  requests.slice().reverse().forEach(function (request) {
    const item = document.createElement("div");
    item.className = "history-item";

    const heading = document.createElement("strong");
    heading.textContent = `SOS #${request.id}`;

    const info = document.createElement("p");
    info.textContent =
      `${request.name} · ${request.phone}\n${request.location}`;

    item.appendChild(heading);
    item.appendChild(info);

    if (request.mapUrl) {
      const link = document.createElement("a");
      link.href = request.mapUrl;
      link.target = "_blank";
      link.rel = "noopener noreferrer";
      link.textContent = "View on Google Maps";
      item.appendChild(link);
    }

    historyList.appendChild(item);
  });
}

sosForm.addEventListener("submit", function (event) {
  event.preventDefault();

  if (!sosForm.reportValidity()) {
    return;
  }

  const name = document.getElementById("victimName").value.trim();
  const phone = document.getElementById("phone").value.trim();
  const location = locationInput.value.trim();
  const details = document.getElementById("details").value.trim();

  if (!name || !location) {
    alert("Please enter your name and emergency location.");
    return;
  }

  const coordinates = getValidCoordinates();

  if (coordinates === false) {
    alert("Please enter valid latitude and longitude values.");
    return;
  }

  const mapUrl = coordinates
    ? `https://www.google.com/maps?q=${coordinates.lat},${coordinates.lon}`
    : "";

  const request = {
    id: nextSOSId++,
    name: name,
    phone: phone,
    location: location,
    latitude: coordinates ? coordinates.lat : null,
    longitude: coordinates ? coordinates.lon : null,
    details: details,
    mapUrl: mapUrl,
    createdAt: new Date().toISOString()
  };

  requests.push(request);
  renderHistory();

  confirmationText.textContent =
    `Demo SOS #${request.id} was created for ${request.name}. ` +
    `This request has not been sent to a rescue team.`;

  if (mapUrl) {
    mapLink.href = mapUrl;
    mapLink.classList.remove("hidden");
  } else {
    mapLink.classList.add("hidden");
  }

  confirmation.classList.remove("hidden");
  confirmation.scrollIntoView({
    behavior: "smooth",
    block: "center"
  });

  console.log("Demo SOS request:", request);
});