// Funciones existentes
function cargarUmbral() {
  fetch("/api/get_threshold.json")
    .then(res => res.json())
    .then(data => {
      document.getElementById("umbralActual").textContent = data.umbral.toFixed(2);
      document.getElementById("umbralInput").value = data.umbral;
    });
}

function actualizarUmbral() {
  const nuevo = parseFloat(document.getElementById("umbralInput").value);
  fetch("/api/set_threshold.json", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ umbral: nuevo })
  })
    .then(res => res.json())
    .then(() => cargarUmbral());
}

function getTemperature() {
    fetch('/api/get_temperatura')
        .then(response => response.json())
        .then(data => {
            console.log("Temperatura:", data.temperature);
            document.getElementById("temp_value").innerText = data.temperature + " °C";
        })
        .catch(error => {
            console.error("Error al obtener la temperatura:", error);
        });
}

function getPotValue() {
    fetch('/api/get_porcentaje_servo')
        .then(response => response.json())
        .then(data => {
            console.log("Potenciómetro:", data.potentiometer);
            document.getElementById("porcentaje").innerText = data.potentiometer + " %";
        })
        .catch(error => {
            console.error("Error al obtener valor del potenciómetro:", error);
        });
}

function actualizarWiFi() {
    const ssid = document.getElementById("ssid").value;
    const password = document.getElementById("pass").value;

    fetch("/api/set_wifi.json", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ ssid, password })
    });
}

function resetAP() {
    fetch('/api/reset_ap', { method: 'POST' })
    .then(res => res.text())
    .then(msg => {
        console.log("Respuesta del servidor (AP):", msg);
        alert("El punto de acceso ha sido restaurado a su configuración por defecto.");
    })
    .catch(err => console.error("Error al resetear AP:", err));
}

function resetSTA() {
    fetch('/api/reset_sta', { method: 'POST' })
    .then(res => res.text())
    .then(msg => {
        console.log("Respuesta del servidor (STA):", msg);
        alert("Las credenciales de la red WiFi han sido eliminadas.");
    })
    .catch(err => console.error("Error al resetear STA:", err));
}


// Cambiar el modo
function setMode(newMode) {
    fetch('/api/set_mode', {
        method: 'POST',
        headers: { 'Content-Type': 'text/plain' },
        body: String(newMode)
    })
    .then(res => res.text())
    .then(msg => {
        console.log("Cambio de Modo: ", msg);
        alert("Respuesta del servidor: " + msg);
        getCurrentMode();  // Forzar actualización
    })
    .catch(err => console.error("Error al cambiar modo:", err));
}


// Obtener el modo actual
function getCurrentMode() {
    fetch('/api/get_mode.json')
        .then(res => res.json())
        .then(data => {
            const modeName = data.name;
            document.getElementById("modoActual").textContent = "Modo actual: " + modeName;
        })
        .catch(err => console.error("Error al obtener modo:", err));
}

let lastMode = null;

function checkModeChange() {
    fetch('/api/get_mode.json')
        .then(res => res.json())
        .then(data => {
            const mode = data.mode;
            const name = data.name;

            if (mode !== lastMode) {
                lastMode = mode;
                document.getElementById("modoActual").textContent = "Modo actual: " + name;
            }
        })
        .catch(err => console.error("Error verificando modo:", err));
}

// Llama esta función periódicamente (cada 2 segundos, por ejemplo)
setInterval(checkModeChange, 2000);




// Nuevas funciones para WiFi AP
function cargarConfigWiFi() {
  fetch("/api/get_ap_config")
    .then(res => res.json())
    .then(data => {
      document.getElementById("ap-ssid").value = data.ap_ssid;
      document.getElementById("ap-password").value = data.ap_password;
    })
    .catch(error => {
      console.error("Error al cargar configuración WiFi:", error);
      mostrarMensaje("Error al cargar configuración WiFi", "error");
    });config
}

function guardarConfigWiFi() {
  const ssid = document.getElementById("ap-ssid").value;
  const password = document.getElementById("ap-password").value;
  
  if (!ssid || ssid.length < 1 || !password || password.length < 8) {
    mostrarMensaje("El SSID debe tener al menos 1 carácter y la contraseña 8 caracteres", "error");
    return;
  }

  fetch("/api/set_ap_config", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ ap_ssid: ssid, ap_password: password })
  })
  .then(res => res.json())
  .then(data => {
    if (data.status === "ok") {
      mostrarMensaje("Configuración WiFi guardada correctamente. Los cambios se aplicarán inmediatamente.", "success");
    } else {
      mostrarMensaje("Error al guardar la configuración WiFi", "error");
    }
  })
  .catch(error => {
    console.error("Error:", error);
    mostrarMensaje("Error de conexión al guardar configuración", "error");
  });
}

function mostrarMensaje(mensaje, tipo) {
  const statusDiv = document.getElementById("wifi-status");
  statusDiv.textContent = mensaje;
  statusDiv.className = `status-message ${tipo}`;
  statusDiv.style.display = "block";
  
  setTimeout(() => {
    statusDiv.style.display = "none";
  }, 5000);
}

// Inicialización
window.onload = () => {
  cargarUmbral();
  cargarConfigWiFi();
  listarRegistros();
};

  // Llamadas periódicas cada 2 segundos
  setInterval(() => {
    getTemperature();
    getPotValue();
  }, 500);