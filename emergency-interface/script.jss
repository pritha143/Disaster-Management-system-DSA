
const homeView = document.getElementById("homeView");
const loginView = document.getElementById("loginView");
const loginToggleBtn = document.getElementById("loginToggleBtn");
const backToHome = document.getElementById("backToHome");
const navBrand = document.getElementById("navBrand");

const sosBtn = document.getElementById("sosBtn");
const emergencyType = document.getElementById("emergencyType");
const sosDialog = document.getElementById("sosDialog");
const confirmationText = document.getElementById("confirmationText");
const cancelSOS = document.getElementById("cancelSOS");
const confirmSOS = document.getElementById("confirmSOS");

const locationBtn = document.getElementById("locationBtn");
const locationStatus = document.getElementById("locationStatus");
const clock = document.getElementById("clock");

const loginForm = document.getElementById("loginForm");
const loginMessage = document.getElementById("loginMessage");
const toast = document.getElementById("toast");

let currentLocation = null;
let toastTimer = null;

function showHome() {
    loginView.classList.add("hidden");
    homeView.classList.remove("hidden");
    loginToggleBtn.classList.remove("hidden");
}

function showLogin() {
    homeView.classList.add("hidden");
    loginView.classList.remove("hidden");
    loginToggleBtn.classList.add("hidden");
}

loginToggleBtn.addEventListener("click", showLogin);

backToHome.addEventListener("click", showHome);

navBrand.addEventListener("click", function (event) {
    event.preventDefault();
    showHome();
});

function updateClock() {
    const now = new Date();

    clock.textContent = now.toLocaleTimeString("en-IN", {
        hour: "2-digit",
        minute: "2-digit",
        second: "2-digit",
        hour12: true
    });
}

updateClock();
setInterval(updateClock, 1000);

function showToast(message) {
    toast.textContent = message;
    toast.classList.remove("hidden");

    clearTimeout(toastTimer);

    toastTimer = setTimeout(function () {
        toast.classList.add("hidden");
    }, 4000);
}

sosBtn.addEventListener("click", function () {
    const category = emergencyType.value;

    confirmationText.textContent =
        "You selected " + category +
        ". Confirm to continue with the demonstration SOS workflow.";

    sosDialog.showModal();
});

cancelSOS.addEventListener("click", function () {
    sosDialog.close();
});

confirmSOS.addEventListener("click", function () {
    const category = emergencyType.value;

    sosDialog.close();

    let message = "Demo SOS confirmed for: " + category + ".";

    if (currentLocation) {
        message += " Browser coordinates are available.";
    } else {
        message += " No location has been shared.";
    }

    message += " No emergency alert was transmitted.";

    showToast(message);
});

locationBtn.addEventListener("click", function () {
    if (!navigator.geolocation) {
        locationStatus.textContent =
            "Geolocation is not supported by this browser.";

        showToast("Your browser does not support location services.");
        return;
    }

    locationStatus.textContent = "Requesting location permission...";
    locationBtn.disabled = true;
    locationBtn.textContent = "Getting location...";

    navigator.geolocation.getCurrentPosition(
        function (position) {
            currentLocation = {
                latitude: position.coords.latitude,
                longitude: position.coords.longitude
            };

            locationStatus.textContent =
                "Location obtained: " +
                currentLocation.latitude.toFixed(4) + ", " +
                currentLocation.longitude.toFixed(4);

            locationBtn.textContent = "Refresh my location";
            locationBtn.disabled = false;

            showToast("Location obtained in this browser.");
        },
        function (error) {
            currentLocation = null;

            if (error.code === error.PERMISSION_DENIED) {
                locationStatus.textContent =
                    "Location permission was denied.";
            } else if (error.code === error.POSITION_UNAVAILABLE) {
                locationStatus.textContent =
                    "Your location is currently unavailable.";
            } else {
                locationStatus.textContent =
                    "Location request timed out. Please try again.";
            }

            locationBtn.textContent = "Share my location";
            locationBtn.disabled = false;
        },
        {
            enableHighAccuracy: true,
            timeout: 10000,
            maximumAge: 0
        }
    );
});

loginForm.addEventListener("submit", function (event) {
    event.preventDefault();

    const username = document.getElementById("username").value.trim();

    if (!username) {
        loginMessage.textContent = "Please enter your officer ID.";
        return;
    }

    loginMessage.textContent =
        "Interface demonstration only. Backend authentication is not configured.";
});
