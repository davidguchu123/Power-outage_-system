# ⚡ PowerWatch Kenya — Power Outage Prediction & Reporting System

> **University OOP Project** | C++ Backend + HTML/CSS/JS Frontend  
> Demonstrates: Encapsulation · Inheritance · Polymorphism · Abstraction

---

## 📁 Project Structure

```
power-outage-system/
│
├── backend/
│   ├── main.cpp          ← Full C++ backend (all OOP classes + CGI API)
│   ├── outage_api        ← Compiled binary (auto-generated)
│   ├── outage_api.cgi    ← CGI-renamed binary (for web servers)
│   └── Makefile          ← Build automation
│
├── frontend/
│   ├── index.html        ← Home page
│   ├── css/
│   │   └── style.css     ← Full stylesheet (dark industrial theme)
│   ├── js/
│   │   ├── api.js        ← Shared API client + mock data layer
│   │   └── home.js       ← Home page JS
│   └── pages/
│       ├── report.html   ← Report Outage page
│       ├── outages.html  ← View All Outages + map simulation
│       ├── predict.html  ← Prediction page
│       └── admin.html    ← Admin Panel
│
├── data/
│   └── reports.dat       ← Pipe-delimited flat-file database
│
├── run.sh                ← One-click startup script
└── README.md             ← This file
```

---

## 🧠 OOP Principles — Where to Find Them

### 🔒 Encapsulation
**File:** `backend/main.cpp` → `class OutageReport`  
All attributes (`location`, `startTime`, `duration`, `status`) are **private**.  
Accessed only through public `getters` and `setters`:
```cpp
class OutageReport {
private:
    char location[100];   // ← PRIVATE
    double duration;
public:
    void setLocation(const char* loc) { strncpy(location, loc, 99); }  // setter
    const char* getLocation() const   { return location; }              // getter
};
```

---

### 🧬 Inheritance
**File:** `backend/main.cpp` → `User → ResidentUser`, `User → AdminUser`
```
User  (base class)
 ├── ResidentUser  : public User
 └── AdminUser     : public User
```
`AdminUser` inherits all `User` members AND adds exclusive methods:
- `viewAllReports()` — not available to `ResidentUser`
- `resolveReport()` — only admins can resolve

---

### 🎭 Polymorphism
**File:** `backend/main.cpp` → virtual `reportOutage()`, `predictOutage()`

```cpp
User* user;
if (strcmp(userType, "admin") == 0)
    user = new AdminUser();    // ← same pointer type
else
    user = new ResidentUser();

user->reportOutage(reports, count);  // ← calls the RIGHT version at runtime
```

At runtime, the correct override is called based on the actual object type.

---

### 🫧 Abstraction
**File:** `backend/main.cpp` → `class PredictionSystem`

```cpp
class PredictionSystem {
public:
    virtual void predictOutage(OutageReport* reports, int count) = 0;  // pure virtual
    virtual ~PredictionSystem() {}
};
```
`PredictionSystem` cannot be instantiated directly.  
`SimplePrediction` extends it and provides the concrete implementation.

---

## 🚀 How to Run (3 Methods)

### Method 1 — One-Click Script (Recommended)
```bash
cd power-outage-system
chmod +x run.sh
./run.sh
```
Then open: **http://localhost:8080/frontend/index.html**

---

### Method 2 — Manual Steps
```bash
# Step 1: Compile the C++ backend
cd power-outage-system/backend
g++ -std=c++11 -O2 -o outage_api main.cpp

# Step 2: Start the web server (from project root)
cd ..
python3 -m http.server 8080

# Step 3: Open in browser
# http://localhost:8080/frontend/index.html
```

---

### Method 3 — Using Make
```bash
cd power-outage-system/backend
make          # compile
make run      # start web server (opens on port 8080)
make demo     # test backend in terminal (no browser needed)
make clean    # remove compiled binary
```

---

### Method 4 — CGI Deployment (Apache/Nginx)
For a real web server with live C++ CGI execution:

```bash
# 1. Compile with CGI name
cd backend
make cgi       # creates outage_api.cgi

# 2. Copy to your cgi-bin directory
sudo cp outage_api.cgi /usr/lib/cgi-bin/
sudo chmod 755 /usr/lib/cgi-bin/outage_api.cgi

# 3. Copy data folder to cgi accessible path
sudo mkdir -p /usr/lib/cgi-bin/data
sudo cp ../data/reports.dat /usr/lib/cgi-bin/data/

# 4. In api.js, set:
#    const USE_MOCK = false;
#    const API_BASE = '/cgi-bin/outage_api.cgi';
```

---

## ⚙️ Requirements

| Requirement | Version |
|------------|---------|
| g++ / GCC  | 7+ (C++11 support) |
| Python     | 3.x (for dev web server) |
| Browser    | Any modern browser |

### Install g++ (if missing)

**Ubuntu/Debian (including WSL):**
```bash
sudo apt update && sudo apt install g++ build-essential
```

**macOS:**
```bash
xcode-select --install
```

**Windows:**
- Use WSL2 (recommended) or install MinGW-w64

---

## 🖥️ Pages & Features

| Page | URL | Features |
|------|-----|----------|
| Home | `/frontend/index.html` | Stats dashboard, live feed, OOP explainer |
| Report | `/frontend/pages/report.html` | Submit outage form, SMS simulation |
| Outages | `/frontend/pages/outages.html` | Table + map simulation with pins |
| Predict | `/frontend/pages/predict.html` | Run C++ prediction algorithm |
| Admin | `/frontend/pages/admin.html` | Resolve reports, class hierarchy diagram |

---

## 📊 Prediction Algorithm

Implemented in `SimplePrediction::predictOutage()`:

```
Count outages for target location
├── ≥ 5 outages → HIGH risk   → SMS: "ALERT: Possible outage in your area"
├── 2–4 outages → MEDIUM risk → SMS: "Notice: Moderate outage risk"
├── 1 outage    → LOW risk
└── 0 outages   → No history
```

Average duration is calculated from resolved outages only.

---

## 💾 Data Storage

Reports are stored in `data/reports.dat` using pipe-delimited format:
```
id|location|reportedBy|startTime|duration|status
1|Nairobi CBD|John Kamau|2024-11-01 08:30|3.50|resolved
```
No STL containers used — plain arrays and file I/O as required.

---

## 📱 Simulated SMS Alerts

When a HIGH-risk prediction is triggered (≥5 outages in an area),  
the system prints/displays:
```
ALERT: Possible outage in your area. Prepare backup power.
```
This is shown in the browser as a toast notification and an SMS box,
and printed to stdout by the C++ backend.

---

## 🔧 Troubleshooting

**"g++ not found"** → Install build-essential (see Requirements above)

**"Permission denied: ./run.sh"** → Run `chmod +x run.sh`

**"Port 8080 already in use"** → The script automatically tries port 8081,
or run: `python3 -m http.server 9000`

**"Data not loading"** → Make sure you're serving from the project root  
(not from inside `frontend/`), so relative paths resolve correctly.

---

## 📝 C++ Classes Summary

| Class | Type | Key Role |
|-------|------|----------|
| `OutageReport` | Concrete | Stores and serializes a single report |
| `PredictionSystem` | **Abstract** | Interface for prediction algorithms |
| `SimplePrediction` | Derived | Frequency-based prediction (extends PredictionSystem) |
| `User` | Base | Common user attributes + virtual reportOutage() |
| `ResidentUser` | Derived | Overrides reportOutage() for residents |
| `AdminUser` | Derived | Overrides reportOutage() + adds admin methods |
| `ReportStorage` | Concrete | File I/O for persisting reports |

---

*PowerWatch Kenya — Developed as a University C++ OOP Project*
