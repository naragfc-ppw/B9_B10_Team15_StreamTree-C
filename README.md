# StreamTree-C

## A Concept-Drift-Aware Incremental Decision-Tree Framework for Embedded Data Streams

## 1. Team Information

- **Batch:** 9 & 10 (Combined)
- **Team Number:** 15
- **Repository Name:** `B9_B10_Team15_StreamTree-C`

### Team Members

1. Kritika Agarwal
2. Utsav
3. Deepak Bhatt
4. Vaibhav Bora

---

## 2. Project Overview

StreamTree-C is a C-based prototype designed to classify continuously arriving data using an incremental decision tree.

Unlike traditional machine-learning approaches that require storing the complete dataset before training, StreamTree-C processes one sample at a time.

The framework combines a Hoeffding-style incremental decision tree with a DDM-inspired drift detector to monitor changes in prediction errors.

The project focuses on streaming analytics, bounded tree growth, online learning and concept-drift monitoring.

---

## 3. Project Objectives

1. Process incoming data samples incrementally without storing the entire stream.
2. Implement a pointer-based incremental decision tree.
3. Use Gini gain and a Hoeffding-bound split decision.
4. Control tree growth using a configurable maximum-node budget.
5. Evaluate predictions using prequential test-then-train evaluation.
6. Monitor prediction errors using a DDM-inspired drift detector.
7. Record experimental results in CSV format.

---

## 4. Implemented Features

- Deterministic SEA-style synthetic stream generator.
- Three numerical input features and binary classification.
- Configurable label noise and concept-change point.
- Pointer-based incremental decision tree.
- Histogram-based candidate split evaluation.
- Gini gain calculation.
- Hoeffding-bound split decision.
- Configurable maximum tree-node budget.
- Prequential test-then-train evaluation.
- Accuracy and macro-F1 calculation.
- DDM-inspired drift monitoring.
- Warning and confirmed drift alarm logging.
- Full-tree reset after a confirmed drift alarm.
- Per-sample CSV result generation.
- Web-based monitoring dashboard.
- Prediction accuracy chart and concept drift timeline.
- Stream record visualization.

**Implementation limitation:** The current adaptation method resets the entire tree after a confirmed drift. Selective subtree replacement, advanced pruning and histogram compression are not yet implemented.

---

## 5. Technology and C Concepts Used

### Programming Technologies

- C (C11)
- GCC Compiler
- Linux/Ubuntu through WSL 2
- Visual Studio Code
- HTML5
- CSS3
- JavaScript
- Chart.js

### C Programming Concepts

- Structures (`struct`)
- Pointers and dynamic memory allocation
- Functions and modular programming
- Header files
- File handling and CSV generation
- Conditional statements and loops
- Mathematical calculations
- Compilation using GCC

---

## 6. Folder Structure

```text
StreamTree-C/
├── src/
│   ├── main.c
│   ├── stream_gen.c
│   ├── stats.c
│   ├── hoeffding_tree.c
│   ├── prequential.c
│   └── drift.c
├── include/
│   ├── stream_gen.h
│   ├── stats.h
│   ├── hoeffding_tree.h
│   ├── prequential.h
│   └── drift.h
├── data/
│   ├── test_stream.csv
│   └── prequential_results.csv
├── results/
├── tests/
│   └── test_drift.c
├── screenshots/
│   ├── 01_compilation.png
│   ├── 02_execution.png
│   ├── 03_csv_output.png
│   ├── 04_main_source_code.png
│   ├── 04_main_source_code_2.png
│   ├── 04_main_source_code_3.png
│   ├── 05_drift_header.png
│   ├── 06_drift_implementation.png
│   ├── 07_prequential_evaluation.png
│   ├── 08_hoeffding_tree.png
│   ├── 09_drift_test.png
│   ├── 10_dashboard_overview.png
│   ├── 11_accuracy_chart.png
│   ├── 12_drift_monitor.png
│   ├── 12_drift_timeline.png
│   └── 13_stream_records.png
├── frontend/
│   ├── index.html
│   ├── style.css
│   └── script.js
├── build/
├── docs/
├── pbl/
├── Makefile
├── build.bat
├── run.bat
├── README.md
└── .gitignore
```

---

## 7. Algorithm and Working

The system follows an incremental learning process:

1. Generate a new sample from the data stream.
2. Pass the sample through the current decision tree.
3. Predict its class label.
4. Compare the prediction with the actual label.
5. Record the prediction error.
6. Update the DDM-inspired drift detector.
7. Train the reached leaf using the new sample.
8. Calculate candidate feature splits using histogram bins and Gini gain.
9. Apply the Hoeffding-bound criterion to decide whether a split is justified.
10. Respect the maximum-node budget when creating new nodes.
11. Reset the entire tree if a confirmed drift alarm occurs.
12. Record the results in a CSV file.

### Prequential Evaluation

The model uses the test-then-train approach:

- First, predict the incoming sample.
- Next, evaluate the prediction.
- Finally, train the model using that sample.

This ensures that each prediction is evaluated before the model learns from that observation.

---

## 8. Drift Detection

StreamTree-C includes a DDM-inspired online error-monitoring mechanism.

The detector:

- Monitors the binary prediction-error stream.
- Maintains a reference error rate and standard deviation.
- Produces a warning when the error statistic exceeds its warning threshold.
- Confirms drift when the statistic exceeds its stronger drift threshold after the minimum observation requirement.
- Triggers a full-tree reset after confirmed drift.

The detector is inspired by the Drift Detection Method (DDM), rather than being presented as a complete implementation of every DDM variant.

---

## 9. Build and Execution Instructions

### Requirements

- GCC compiler
- VS Code (recommended)
- Ubuntu or another GCC-compatible environment

### Linux / Ubuntu (WSL 2)

Open the Ubuntu terminal and navigate to the project directory:

```bash
cd /mnt/c/Users/22kri/OneDrive/Desktop/StreamTree-C
```

Compile the project:

```bash
make
```

Run the program:

```bash
./streamtree
```

### Manual Compilation

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -O2 -Iinclude src/main.c src/stream_gen.c src/stats.c src/hoeffding_tree.c src/prequential.c src/drift.c -o streamtree -lm
```

### Windows

With GCC/MinGW installed and available in PATH:

```powershell
.\build.bat
.\run.bat
```

**Note:** On systems where Windows application-control policy blocks locally compiled executables, the Linux/Ubuntu WSL 2 execution method can be used instead.

---

## 10. Command-Line Options

| Option | Description | Default |
|---|---|---|
| `--samples N` | Number of samples | 3000 |
| `--drift-at N` | Concept-change sample index | 1500 |
| `--seed N` | Random seed | 42 |
| `--noise P` | Label noise probability | 0.05 |
| `--max-nodes N` | Maximum tree nodes | 101 |
| `--grace N` | Samples between split attempts | 100 |
| `--output FILE` | CSV output location | `data/prequential_results.csv` |

---

## 11. Experimental Results

The following results were obtained from a run with 3,000 samples, seed 42, noise rate 0.05 and a configured concept change at sample 1500.

| Metric | Observed Result |
|---|---:|
| Samples evaluated | 3000 |
| Random seed | 42 |
| Noise rate | 0.05 |
| Configured drift point | 1500 |
| Prequential accuracy | 71.10% |
| Macro-F1 | 0.6751 |
| Tree nodes | 5 / 101 |
| DDM warning observations | 855 |
| Confirmed drift alarms | 0 |
| First confirmed drift alarm | None |

### Result Interpretation

The experiment generated 855 warning observations but no confirmed drift alarm.

This indicates that the detector did not cross its confirmed-drift threshold during this particular experiment, even though the synthetic generator changed concept at sample 1500.

A separate controlled unit test successfully detected an injected drift at sample 120.

These two results are reported separately because successful detection in a controlled test does not guarantee detection of every concept transition in a continuous stream.

---

## 12. CSV Output

The program generates:

`data/prequential_results.csv`

The CSV includes:

- Sample index
- Input features (`x1`, `x2`, `x3`)
- Actual class
- Predicted class
- Correctness indicator
- Concept ID
- Current tree node count
- DDM status (`NONE`, `WARNING`, `DRIFT`)

---

## 13. Testing and Validation

### Drift Detector Unit Test

Compile the test:

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -O2 -Iinclude tests/test_drift.c src/drift.c -o test_drift -lm
```

Run the test:

```bash
./test_drift
```

**Observed output:**

```text
Controlled drift detection sample: 120
```

### Other Validation Checks

- C11 compilation with warning flags.
- Reproducibility using a fixed random seed.
- CSV output generation.
- Maximum-node budget verification.
- Controlled drift detection test.

---

## 14. Frontend Dashboard

StreamTree-C includes a web-based monitoring dashboard built using HTML, CSS and JavaScript. It reads the prequential evaluation CSV generated by the C program and presents the results in a visual interface.

### Dashboard Features

- Metric cards for total samples, prequential accuracy, macro-F1 score and tree nodes.
- Prediction accuracy chart.
- Drift monitoring panel showing warnings, confirmed alarms and the known drift point.
- Concept drift timeline.
- Stream records table displaying recent predictions.
- Automatic CSV refresh every 10 seconds.

### Running the Dashboard

**Step 1: Build and run the C program**

```bash
make
./streamtree
```

**Step 2: Open another Ubuntu terminal and navigate to the project folder**

```bash
cd /mnt/c/Users/22kri/OneDrive/Desktop/StreamTree-C
```

**Step 3: Start the local web server**

```bash
python3 -m http.server 8000
```

**Step 4: Open the dashboard in your browser**

```text
http://localhost:8000/frontend/
```

**Note:** The dashboard reads the generated CSV file. It refreshes the displayed data automatically, but the C program must be run separately to generate new results.

### Frontend Technologies

- HTML5
- CSS3
- JavaScript
- Chart.js

---

## 15. Screenshots and Demonstration

The `screenshots/` directory contains visual evidence of the implementation and testing process.

### C Implementation and Testing

1. Successful compilation.
2. Program execution.
3. CSV experiment output.
4. Main source code.
5. Drift detector header.
6. Drift detector implementation.
7. Prequential evaluation.
8. Hoeffding tree implementation.
9. Controlled drift detector test.

### Frontend Dashboard

10. Dashboard overview.
11. Prediction accuracy chart.
12. Drift monitoring panel.
13. Stream records table.

---

## 16. Current Limitations

- The current synthetic generator focuses on SEA-style binary classification.
- The drift detector may produce warnings without confirming drift.
- Full-tree reset is used instead of selective subtree replacement.
- Advanced memory compression and pruning are not implemented.
- Baseline model comparisons are pending.
- Multi-seed statistical evaluation is pending.
- The dashboard currently reads CSV files through a local web server; direct integration with a C HTTP API is not yet implemented.

---

## 17. Future Development

1. Evaluate drift detection under abrupt, gradual and recurring concept changes.
2. Compare full-tree reset with no adaptation and selective subtree replacement.
3. Introduce memory telemetry and pruning strategies.
4. Implement baseline models such as Naive Bayes and sliding-window logistic regression.
5. Conduct multi-seed experiments and statistical comparisons.
6. Integrate the dashboard with a direct C-based streaming interface.

---

## 18. Research Integrity

The reported performance values correspond to the specific experiment described in this README.

No claims of superiority over existing algorithms, guaranteed drift detection, improved recovery time or reduced memory consumption are made without further experiments and comparative evaluation.

The project is an evolving prototype intended to demonstrate incremental learning, bounded tree growth and concept-drift monitoring using C.

---

**StreamTree-C | Batch 9 & 10 | Team 15**