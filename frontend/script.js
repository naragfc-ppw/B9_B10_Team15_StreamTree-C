
const CSV_PATH = "../data/prequential_results.csv";
const MAX_NODE_BUDGET = 101;
const KNOWN_DRIFT_POINT = 1500;
const ROLLING_WINDOW = 100;

let accuracyChart = null;

const $ = (id) => document.getElementById(id);

function showToast(message) {
    const toast = $("toast");
    toast.textContent = message;
    toast.classList.add("show");

    setTimeout(() => toast.classList.remove("show"), 3000);
}

function parseCSV(text) {
    const lines = text.trim().split(/\r?\n/);

    if (lines.length < 2) {
        throw new Error("CSV contains no sample records.");
    }

    const headers = lines[0].split(",").map(h => h.trim());

    return lines.slice(1).filter(Boolean).map(line => {
        const values = line.split(",");
        const row = {};

        headers.forEach((header, index) => {
            row[header] = values[index]?.trim() ?? "";
        });

        return row;
    });
}

function calculateMetrics(rows) {
    const total = rows.length;

    let correct = 0;
    let truePositive = 0;
    let falsePositive = 0;
    let falseNegative = 0;
    let trueNegative = 0;

    rows.forEach(row => {
        const actual = Number(row.actual);
        const predicted = Number(row.predicted);

        if (Number(row.correct) === 1) correct++;

        if (actual === 1 && predicted === 1) truePositive++;
        else if (actual === 0 && predicted === 1) falsePositive++;
        else if (actual === 1 && predicted === 0) falseNegative++;
        else if (actual === 0 && predicted === 0) trueNegative++;
    });

    const accuracy = total ? correct / total : 0;

    function f1(tp, fp, fn) {
        const precision = tp + fp === 0 ? 0 : tp / (tp + fp);
        const recall = tp + fn === 0 ? 0 : tp / (tp + fn);

        return precision + recall === 0
            ? 0
            : 2 * precision * recall / (precision + recall);
    }

    const f1Class0 = f1(trueNegative, falseNegative, falsePositive);
    const f1Class1 = f1(truePositive, falsePositive, falseNegative);
    const macroF1 = (f1Class0 + f1Class1) / 2;

    return { total, accuracy, macroF1 };
}

function rollingAccuracy(rows) {
    const result = [];
    let correctCount = 0;

    rows.forEach((row, index) => {
        if (Number(row.correct) === 1) correctCount++;

        const start = Math.max(0, index - ROLLING_WINDOW + 1);
        const windowRows = rows.slice(start, index + 1);
        const windowCorrect = windowRows.reduce(
            (sum, item) => sum + (Number(item.correct) === 1 ? 1 : 0),
            0
        );

        result.push({
            sample: Number(row.sample),
            accuracy: (windowCorrect / windowRows.length) * 100
        });
    });

    return result;
}

function updateChart(rows) {
    const chartData = rollingAccuracy(rows);

    const labels = chartData.map(item => item.sample);
    const values = chartData.map(item => item.accuracy);

    if (accuracyChart) {
        accuracyChart.data.labels = labels;
        accuracyChart.data.datasets[0].data = values;
        accuracyChart.update();
        return;
    }

    accuracyChart = new Chart($("accuracyChart"), {
        type: "line",
        data: {
            labels,
            datasets: [{
                label: "Rolling Accuracy",
                data: values,
                borderColor: "#5798ff",
                backgroundColor: "rgba(87, 152, 255, 0.12)",
                fill: true,
                tension: 0.3,
                pointRadius: 0,
                pointHoverRadius: 4,
                borderWidth: 2
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            interaction: {
                intersect: false,
                mode: "index"
            },
            plugins: {
                legend: {
                    display: false
                }
            },
            scales: {
                x: {
                    grid: { color: "#24334a" },
                    ticks: {
                        color: "#8998b1",
                        maxTicksLimit: 8
                    },
                    title: {
                        display: true,
                        text: "Sample Number",
                        color: "#8998b1"
                    }
                },
                y: {
                    min: 0,
                    max: 100,
                    grid: { color: "#24334a" },
                    ticks: {
                        color: "#8998b1",
                        callback: value => value + "%"
                    }
                }
            }
        }
    });
}

function updateDrift(rows) {
    const warnings = rows.filter(
        row => row.ddm_status.toUpperCase() === "WARNING"
    );

    const alarms = rows.filter(
        row => row.ddm_status.toUpperCase() === "DRIFT"
    );

    const firstAlarm = alarms.length
        ? alarms[0].sample
        : "None";

    $("warnings").textContent = warnings.length;
    $("alarms").textContent = alarms.length;
    $("firstAlarm").textContent = firstAlarm;
    $("driftPoint").textContent = KNOWN_DRIFT_POINT;

    const status = $("driftStatus");

    if (alarms.length > 0) {
        status.innerHTML = `
            <div class="drift-symbol"
                 style="background:#ff858522;color:#ff8585">!</div>
            <h2 style="color:#ff8585">Drift Detected</h2>
            <p>${alarms.length} confirmed drift alarm(s) recorded.</p>
        `;
    } else if (warnings.length > 0) {
        status.innerHTML = `
            <div class="drift-symbol"
                 style="background:#ffb15c22;color:#ffb15c">!</div>
            <h2 style="color:#ffb15c">Warning Observed</h2>
            <p>No confirmed drift alarm in this dataset.</p>
        `;
    } else {
        status.innerHTML = `
            <div class="drift-symbol">✓</div>
            <h2>No Drift Alarm</h2>
            <p>No warning or confirmed drift status recorded.</p>
        `;
    }
}

function updateRecords(rows) {
    const body = $("recordsBody");
    body.replaceChildren();

    const latestRows = [...rows].slice(-12).reverse();

    latestRows.forEach(row => {
        const tr = document.createElement("tr");

        const values = [
            row.sample,
            row.actual,
            row.predicted,
            Number(row.correct) === 1 ? "Correct" : "Incorrect",
            row.concept_id,
            row.nodes,
            row.ddm_status
        ];

        values.forEach((value, index) => {
            const td = document.createElement("td");
            td.textContent = value;

            if (index === 3) {
                td.className = Number(row.correct) === 1
                    ? "result-good"
                    : "result-bad";
            }

            tr.appendChild(td);
        });

        body.appendChild(tr);
    });
}

function updateDashboard(rows) {
    const metrics = calculateMetrics(rows);
    const lastRow = rows[rows.length - 1];

    $("totalSamples").textContent =
        metrics.total.toLocaleString();

    $("accuracy").textContent =
        (metrics.accuracy * 100).toFixed(2) + "%";

    $("macroF1").textContent =
        (metrics.macroF1 * 100).toFixed(2) + "%";

    $("treeNodes").textContent =
        `${lastRow.nodes} / ${MAX_NODE_BUDGET}`;

    $("lastUpdated").textContent =
        "Last updated: " + new Date().toLocaleTimeString();

    updateChart(rows);
    updateDrift(rows);
    updateRecords(rows);
    updateTimeline(rows);
}

async function loadDashboard() {
    try {
        $("refreshBtn").textContent = "↻ Loading...";

        const response = await fetch(
            CSV_PATH + "?t=" + Date.now()
        );

        if (!response.ok) {
            throw new Error("Could not load the CSV file.");
        }

        const csvText = await response.text();
        const rows = parseCSV(csvText);

        updateDashboard(rows);
        showToast("Dashboard data loaded successfully.");

    } catch (error) {
        console.error(error);

        showToast(
            "Unable to load CSV. Check the local server and data file."
        );

    } finally {
        $("refreshBtn").textContent = "↻ Refresh Data";
    }
}

$("refreshBtn").addEventListener("click", loadDashboard);

// Load dashboard when the page opens
loadDashboard();

// Check for updated CSV data every 10 seconds
setInterval(loadDashboard, 10000);


function updateTimeline(rows) {
    const track = $("timelineTrack");
    track.replaceChildren();

    const total = rows.length;
    const warnings = rows.filter(
        row => row.ddm_status.toUpperCase() === "WARNING"
    );
    const alarms = rows.filter(
        row => row.ddm_status.toUpperCase() === "DRIFT"
    );

    const line = document.createElement("div");
    line.className = "timeline-line";
    track.appendChild(line);

    function addEvent(sample, title, description, type, position) {
        const event = document.createElement("div");
        event.className = "timeline-event";

        event.style.left = `${position}%`;

        const marker = document.createElement("span");
        marker.className = `timeline-marker ${type}`;

        const heading = document.createElement("strong");
        heading.textContent = title;

        const detail = document.createElement("small");
        detail.textContent = description;

        event.append(marker, heading, detail);
        track.appendChild(event);
    }

    if (warnings.length > 0) {
        const first = warnings[0].sample;

        addEvent(
            first,
            `Sample ${first}`,
            `${warnings.length} warning observations`,
            "warning",
            20
        );
    }

    if (total > 0) {
        addEvent(
            KNOWN_DRIFT_POINT,
            `Sample ${KNOWN_DRIFT_POINT}`,
            "Known injected concept change",
            "known",
            55
        );
    }

    if (alarms.length > 0) {
        const first = alarms[0].sample;

        addEvent(
            first,
            `Sample ${first}`,
            `${alarms.length} confirmed alarm(s)`,
            "alarm",
            85
        );
    } else {
        addEvent(
            total - 1,
            "No confirmed alarm",
            "Detector did not confirm drift",
            "warning",
            85
        );
    }
}
