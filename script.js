const BACKEND_URL = 'https://collins-classified-experts-material.trycloudflare.com';

const homeView = document.getElementById('homeView');
const categoryView = document.getElementById('categoryView');
const confirmationView = document.getElementById('confirmationView');
const loginView = document.getElementById('loginView');
const dashboardView = document.getElementById('dashboardView');

const sosBtn = document.getElementById('sosBtn');
const loginToggleBtn = document.getElementById('loginToggleBtn');
const logoutBtn = document.getElementById('logoutBtn');
const userBadge = document.getElementById('userBadge');
const officerBadgeName = document.getElementById('officerBadgeName');
const backToHome = document.getElementById('backToHome');
const backToSos = document.getElementById('backToSos');
const navBrand = document.getElementById('navBrand');
const cancelAlertBtn = document.getElementById('cancelAlertBtn');

const loginForm = document.getElementById('loginForm');
const usernameInput = document.getElementById('username');
const passwordInput = document.getElementById('password');
const loginLatInput = document.getElementById('loginLat');
const loginLonInput = document.getElementById('loginLon');
const fetchLoginGpsBtn = document.getElementById('fetchLoginGpsBtn');
const authMessage = document.getElementById('authMessage');

const volunteerMeta = document.getElementById('volunteerMeta');
const taskCountEl = document.getElementById('taskCount');
const nextQueueList = document.getElementById('nextQueueList');
const taskListEl = document.getElementById('taskList');
const priorityQueueFullList = document.getElementById('priorityQueueFullList');
const volunteersListEl = document.getElementById('volunteersList');
const refreshDashboardBtn = document.getElementById('refreshDashboardBtn');

const sectionViewSos = document.getElementById('sectionViewSos');
const sectionNextPriority = document.getElementById('sectionNextPriority');
const sectionVolunteers = document.getElementById('sectionVolunteers');

const selectedCategoryDisplay = document.getElementById('selectedCategoryDisplay');
const coordsDisplay = document.getElementById('coordsDisplay');
const locationStatus = document.getElementById('locationStatus');

let currentUser = null;
let currentSosId = null;

function getPriorityForType(type) {
  const normalized = (type || '').toLowerCase();
  if (normalized.includes('fire') || normalized.includes('hazard')) return 1;
  if (normalized.includes('medical')) return 2;
  if (normalized.includes('crime') || normalized.includes('police')) return 3;
  return 4;
}

function hideAllViews() {
  homeView.classList.add('hidden');
  categoryView.classList.add('hidden');
  confirmationView.classList.add('hidden');
  loginView.classList.add('hidden');
  dashboardView.classList.add('hidden');
}

function showHome() {
  hideAllViews();
  homeView.classList.remove('hidden');
  updateNavUI();
}

function showCategoryView() {
  hideAllViews();
  categoryView.classList.remove('hidden');
  updateNavUI();
}

function showLogin() {
  hideAllViews();
  loginView.classList.remove('hidden');
  clearAuthMessage();
  updateNavUI();
}

async function showDashboard() {
  hideAllViews();
  dashboardView.classList.remove('hidden');
  updateNavUI();
  if (currentUser) {
    volunteerMeta.textContent = `${currentUser.name} (ID #${currentUser.id}) • Coords: ${Number(currentUser.latitude).toFixed(4)}, ${Number(currentUser.longitude).toFixed(4)}`;
  }
  await loadDashboardData();
}

function showConfirmation(categoryName) {
  hideAllViews();
  confirmationView.classList.remove('hidden');
  selectedCategoryDisplay.textContent = categoryName;
  coordsDisplay.textContent = 'Fetching coordinates...';
  locationStatus.textContent = 'Acquiring high-accuracy position...';
  updateNavUI();
}

function updateNavUI() {
  if (currentUser) {
    loginToggleBtn.classList.add('hidden');
    userBadge.classList.remove('hidden');
    officerBadgeName.textContent = `${currentUser.name} (#${currentUser.id})`;
  } else {
    userBadge.classList.add('hidden');
    if (!loginView.classList.contains('hidden')) {
      loginToggleBtn.classList.add('hidden');
    } else {
      loginToggleBtn.classList.remove('hidden');
    }
  }
}

function setAuthMessage(msg, type = 'error') {
  authMessage.textContent = msg;
  authMessage.className = `auth-message ${type}`;
  authMessage.classList.remove('hidden');
}

function clearAuthMessage() {
  authMessage.textContent = '';
  authMessage.classList.add('hidden');
}

function fetchCurrentGps(onSuccess, onFailure) {
  if (!('geolocation' in navigator)) {
    if (onFailure) onFailure('Geolocation is not supported in this browser.');
    return;
  }

  navigator.geolocation.getCurrentPosition(
    (pos) => {
      onSuccess(pos.coords.latitude, pos.coords.longitude, pos.coords.accuracy);
    },
    (err) => {
      if (onFailure) onFailure(err.message || 'Could not fetch GPS coordinates.');
    },
    { enableHighAccuracy: true, timeout: 8000, maximumAge: 0 }
  );
}

fetchLoginGpsBtn.addEventListener('click', () => {
  fetchLoginGpsBtn.textContent = 'Fetching GPS...';
  fetchCurrentGps(
    (lat, lon) => {
      loginLatInput.value = lat.toFixed(6);
      loginLonInput.value = lon.toFixed(6);
      fetchLoginGpsBtn.textContent = '📍 Fetch Current GPS Coordinates';
      setAuthMessage(`GPS updated: ${lat.toFixed(6)}, ${lon.toFixed(6)}`, 'success');
    },
    (msg) => {
      fetchLoginGpsBtn.textContent = '📍 Fetch Current GPS Coordinates';
      setAuthMessage(msg, 'error');
    }
  );
});

async function submitLoginRequest(name, password, latVal, lonVal) {
  const payload = { name, password };
  if (!Number.isNaN(latVal) && !Number.isNaN(lonVal) && loginLatInput.value !== '' && loginLonInput.value !== '') {
    payload.latitude = latVal;
    payload.longitude = lonVal;
  }

  try {
    const res = await fetch(`${BACKEND_URL}/api/login`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload)
    });
    const data = await res.json();

    if (!res.ok || !data.success) {
      setAuthMessage(data.error || 'Invalid credentials.', 'error');
      return;
    }

    currentUser = data.user;
    setAuthMessage('Login verified! Opening dashboard...', 'success');
    setTimeout(() => {
      loginForm.reset();
      clearAuthMessage();
      showDashboard();
    }, 500);
  } catch (err) {
    console.error('Login error:', err);
    setAuthMessage('Cannot connect to C++ backend.', 'error');
  }
}

loginForm.addEventListener('submit', (e) => {
  e.preventDefault();
  const name = usernameInput.value.trim();
  const password = passwordInput.value.trim();

  if (!name || !password) {
    setAuthMessage('Please enter both name and password.', 'error');
    return;
  }

  const parsedLat = parseFloat(loginLatInput.value);
  const parsedLon = parseFloat(loginLonInput.value);

  if (loginLatInput.value.trim() === '' || loginLonInput.value.trim() === '') {
    fetchCurrentGps(
      (lat, lon) => {
        loginLatInput.value = lat.toFixed(6);
        loginLonInput.value = lon.toFixed(6);
        submitLoginRequest(name, password, lat, lon);
      },
      () => {
        submitLoginRequest(name, password, NaN, NaN);
      }
    );
  } else {
    submitLoginRequest(name, password, parsedLat, parsedLon);
  }
});

logoutBtn.addEventListener('click', () => {
  currentUser = null;
  showHome();
});

function renderSosCard(item, showDistance = true) {
  const lat = Number(item.latitude).toFixed(6);
  const lon = Number(item.longitude).toFixed(6);
  const mapsUrl = `https://www.google.com/maps?q=${lat},${lon}`;
  const isUrgent = Number(item.priority) <= 2;

  return `
    <div class="task-card ${isUrgent ? 'urgent' : ''}">
      <div class="task-header">
        <span class="task-id">SOS #${item.sosId} • Priority ${item.priority}</span>
        <span class="task-tag medical">${item.type}</span>
      </div>
      <h4 class="task-title">${item.type}</h4>
      <p class="task-details">
        Coords: ${lat}, ${lon}
        ${showDistance && typeof item.distanceKm === 'number' ? ` • Distance: ${Number(item.distanceKm).toFixed(2)} km` : ''}
      </p>
      <div class="task-actions">
        <button type="button" class="action-btn accept" onclick="resolveSos(${item.sosId})">Resolve SOS</button>
        <a class="action-btn info" href="${mapsUrl}" target="_blank" rel="noopener noreferrer">View Map</a>
      </div>
    </div>
  `;
}

async function resolveSos(sosId) {
  try {
    const res = await fetch(`${BACKEND_URL}/api/sos/resolve`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        sosId: sosId,
        resolvedBy: currentUser ? currentUser.name : 'Volunteer'
      })
    });
    const data = await res.json();
    if (!res.ok || !data.success) {
      alert(data.error || 'Could not resolve SOS.');
      return;
    }
    await loadDashboardData();
  } catch (err) {
    console.error('Resolve error:', err);
    alert('Failed to reach backend while resolving SOS.');
  }
}

async function loadDashboardData() {
  const volQuery = currentUser ? `?volunteerId=${currentUser.id}` : '';

  try {
    const [sosRes, nextRes, volRes] = await Promise.all([
      fetch(`${BACKEND_URL}/api/sos${volQuery}`),
      fetch(`${BACKEND_URL}/api/sos/for-volunteer${volQuery}`),
      fetch(`${BACKEND_URL}/api/volunteers`)
    ]);

    const allSos = sosRes.ok ? await sosRes.json() : [];
    const nextQueue = nextRes.ok ? await nextRes.json() : [];
    const volunteers = volRes.ok ? await volRes.json() : [];

    taskCountEl.textContent = allSos.length;

    if (nextQueue.length === 0) {
      nextQueueList.innerHTML = '<p class="empty-msg">No queued SOS requests for your distance/priority right now.</p>';
      priorityQueueFullList.innerHTML = '<p class="empty-msg">No queued SOS requests available.</p>';
    } else {
      nextQueueList.innerHTML = renderSosCard(nextQueue[0], true);
      priorityQueueFullList.innerHTML = nextQueue.map((s) => renderSosCard(s, true)).join('');
    }

    if (allSos.length === 0) {
      taskListEl.innerHTML = '<p class="empty-msg">No active SOS alerts in queue.</p>';
    } else {
      taskListEl.innerHTML = allSos.map((s) => renderSosCard(s, Boolean(currentUser))).join('');
    }

    if (volunteers.length === 0) {
      volunteersListEl.innerHTML = '<p class="empty-msg">No volunteers registered yet.</p>';
    } else {
      volunteersListEl.innerHTML = volunteers
        .map(
          (v) => `
          <div class="task-card">
            <div class="task-header">
              <span class="task-id">Volunteer #${v.id}</span>
              <span class="task-tag disaster">Active Responder</span>
            </div>
            <h4 class="task-title">${v.name}</h4>
            <p class="task-details">Phone: ${v.phone} • Coords: ${Number(v.latitude).toFixed(6)}, ${Number(v.longitude).toFixed(6)}</p>
          </div>
        `
        )
        .join('');
    }
  } catch (err) {
    console.error('Dashboard load failed:', err);
    taskListEl.innerHTML = '<p class="empty-msg">Could not load SOS data from backend.</p>';
  }
}

document.querySelectorAll('.taskbar-btn').forEach((btn) => {
  btn.addEventListener('click', () => {
    document.querySelectorAll('.taskbar-btn').forEach((b) => b.classList.remove('active'));
    btn.classList.add('active');

    const tab = btn.getAttribute('data-tab');
    sectionViewSos.classList.toggle('hidden', tab !== 'viewSos');
    sectionNextPriority.classList.toggle('hidden', tab !== 'nextPriority');
    sectionVolunteers.classList.toggle('hidden', tab !== 'volunteers');
  });
});

refreshDashboardBtn.addEventListener('click', loadDashboardData);

async function sendSOSToBackend(type, priority, latitude, longitude) {
  const payload = {
    type: type,
    priority: priority,
    latitude: latitude,
    longitude: longitude
  };

  try {
    const response = await fetch(`${BACKEND_URL}/api/sos`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload)
    });

    if (!response.ok) {
      throw new Error(`Server responded with ${response.status}`);
    }

    const result = await response.json();
    console.log('Backend response:', result);
    if (result && result.sosId) {
      currentSosId = result.sosId;
    }
    locationStatus.textContent = result.sosId
      ? `Logged with C++ backend (SOS #${result.sosId})`
      : 'Sent to C++ backend.';
  } catch (err) {
    console.error('Could not reach C++ backend:', err);
    locationStatus.textContent = 'Shown locally (C++ server not connected yet).';
  }
}

function triggerSOS(type) {
  const priority = getPriorityForType(type);
  currentSosId = null;
  showConfirmation(type);

  if (!('geolocation' in navigator)) {
    coordsDisplay.textContent = 'Not Supported';
    locationStatus.textContent = 'Geolocation is not supported by your browser.';
    return;
  }

  navigator.geolocation.getCurrentPosition(
    (position) => {
      const rawLat = position.coords.latitude;
      const rawLon = position.coords.longitude;
      const formattedLat = rawLat.toFixed(6);
      const formattedLon = rawLon.toFixed(6);

      coordsDisplay.textContent = `${formattedLat}, ${formattedLon}`;
      locationStatus.textContent = `Accuracy: ~${Math.round(position.coords.accuracy)}m • Sending SOS...`;

      sendSOSToBackend(type, priority, rawLat, rawLon);
    },
    (error) => {
      coordsDisplay.textContent = 'Location Unavailable';
      switch (error.code) {
        case error.PERMISSION_DENIED:
          locationStatus.textContent = 'Location access denied by user.';
          break;
        case error.POSITION_UNAVAILABLE:
          locationStatus.textContent = 'Position information unavailable.';
          break;
        case error.TIMEOUT:
          locationStatus.textContent = 'Location request timed out.';
          break;
        default:
          locationStatus.textContent = 'Failed to fetch coordinates.';
      }
    },
    { enableHighAccuracy: true, timeout: 10000, maximumAge: 0 }
  );
}

sosBtn.addEventListener('click', (e) => {
  e.preventDefault();
  showCategoryView();
});

document.querySelectorAll('.emergency-option').forEach((button) => {
  button.addEventListener('click', (e) => {
    e.preventDefault();
    const type = button.getAttribute('data-type');
    triggerSOS(type);
  });
});

loginToggleBtn.addEventListener('click', () => {
  if (currentUser) {
    showDashboard();
  } else {
    showLogin();
  }
});

backToHome.addEventListener('click', (e) => {
  e.preventDefault();
  showHome();
});

backToSos.addEventListener('click', (e) => {
  e.preventDefault();
  showHome();
});

cancelAlertBtn.addEventListener('click', async () => {
  if (currentSosId !== null) {
    try {
      const response = await fetch(`${BACKEND_URL}/api/sos/${currentSosId}`, {
        method: 'DELETE'
      });
      if (!response.ok) {
        throw new Error(`Server responded with ${response.status}`);
      }
      const result = await response.json();
      console.log('Cancel response:', result);
      currentSosId = null;
    } catch (err) {
      console.error('Failed to cancel SOS on server:', err);
    }
  }

  alert('Emergency request cancelled.');
  showHome();
});

navBrand.addEventListener('click', (e) => {
  e.preventDefault();
  if (currentUser) {
    showDashboard();
  } else {
    showHome();
  }
});