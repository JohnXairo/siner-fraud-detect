// dashboard.js — polling de estado del motor y auto-refresh
(function () {
    var ENGINE_URL = 'http://localhost:8081';

    function checkEngine() {
        var el = document.getElementById('engineStatus');
        if (!el) return;

        fetch(ENGINE_URL + '/health', { mode: 'cors' })
            .then(function (r) { return r.json(); })
            .then(function (d) {
                el.innerHTML =
                    '<span class="dot dot-ok"></span> Motor activo &mdash; ' +
                    d.service + ' v' + d.version;
            })
            .catch(function () {
                el.innerHTML =
                    '<span class="dot dot-error"></span> Motor no disponible';
            });
    }

    function setPreset(amount, country) {
        var amountInput  = document.querySelector('input[name="amount"]');
        var countryInput = document.querySelector('select[name="country"]');
        if (amountInput)  amountInput.value  = amount;
        if (countryInput) countryInput.value = country;
    }

    // Exponer setPreset globalmente para los botones del formulario
    window.setPreset = setPreset;

    // Verificar estado del motor al cargar y cada 30 segundos
    checkEngine();
    setInterval(checkEngine, 30000);
})();
