#!/bin/bash
# ============================================================
# run.sh — PowerWatch Kenya — One-Click Startup Script
# ============================================================
# Usage:  ./run.sh
# ============================================================

set -e

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
BACKEND_DIR="$ROOT_DIR/backend"
PORT=8080

# ---- Colours ----
RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'
CYAN='\033[0;36m'; BOLD='\033[1m'; NC='\033[0m'

echo -e "${CYAN}"
echo "  ⚡  PowerWatch Kenya — Power Outage Prediction System"
echo "  ======================================================"
echo -e "${NC}"

# ---- Step 1: Compile C++ backend ----
echo -e "${BOLD}[1/3] Compiling C++ backend...${NC}"
cd "$BACKEND_DIR"

if g++ -std=c++11 -O2 -Wall -o outage_api main.cpp 2>&1; then
    echo -e "${GREEN}      ✅  C++ backend compiled successfully${NC}"
    cp outage_api outage_api.cgi
else
    echo -e "${RED}      ❌  Compilation failed. Check g++ is installed.${NC}"
    exit 1
fi
cd "$ROOT_DIR"

# ---- Step 2: Verify data file ----
echo -e "${BOLD}[2/3] Checking data directory...${NC}"
if [ ! -f "$ROOT_DIR/data/reports.dat" ]; then
    echo -e "${YELLOW}      ⚠️  No data file found — creating sample data${NC}"
    mkdir -p "$ROOT_DIR/data"
    cat > "$ROOT_DIR/data/reports.dat" << 'EOF'
1|Nairobi CBD|John Kamau|2024-11-01 08:30|3.50|resolved
2|Westlands|Mary Wanjiku|2024-11-03 14:00|2.00|resolved
3|Nairobi CBD|Peter Otieno|2024-11-05 09:15|4.00|resolved
4|Kisumu|Alice Achieng|2024-11-06 11:00|1.50|resolved
5|Nairobi CBD|Samuel Njoroge|2024-11-08 07:45|5.00|resolved
6|Westlands|Grace Muthoni|2024-11-10 16:30|3.00|resolved
7|Mombasa|Hassan Omar|2024-11-11 13:00|2.50|resolved
8|Nairobi CBD|Faith Karimi|2024-11-13 08:00|6.00|resolved
9|Kisumu|Tom Ochieng|2024-11-14 10:30|1.00|resolved
10|Westlands|Diana Chebet|2024-11-15 15:00|2.00|resolved
11|Nairobi CBD|Mike Gitonga|2024-11-16 09:00|3.00|active
12|Mombasa|Fatuma Ali|2024-11-17 11:45|0.00|active
13|Nakuru|James Ndegwa|2024-11-17 14:00|0.00|active
EOF
fi
echo -e "${GREEN}      ✅  Data file ready ($(wc -l < "$ROOT_DIR/data/reports.dat") records)${NC}"

# ---- Step 3: Quick backend API test ----
echo -e "${BOLD}[3/3] Testing backend API...${NC}"
cd "$BACKEND_DIR"
STATS=$(QUERY_STRING="action=stats" REQUEST_METHOD=GET ./outage_api 2>/dev/null \
        | python3 -c "import sys; print(sys.stdin.read().split('\r\n\r\n',1)[-1])" 2>/dev/null)
if echo "$STATS" | grep -q "totalReports"; then
    TOTAL=$(echo "$STATS" | python3 -c "import sys,json; d=json.load(sys.stdin); print(d['totalReports'])" 2>/dev/null)
    echo -e "${GREEN}      ✅  Backend OK — ${TOTAL} reports loaded${NC}"
else
    echo -e "${YELLOW}      ⚠️  Backend test inconclusive (may still work)${NC}"
fi
cd "$ROOT_DIR"

# ---- Launch web server ----
echo ""
echo -e "${BOLD}Starting web server on port ${PORT}...${NC}"
echo ""
echo -e "  ${CYAN}🌐  Open in browser:${NC}"
echo -e "  ${BOLD}  → http://localhost:${PORT}/frontend/index.html${NC}"
echo ""
echo -e "  ${CYAN}📄  Pages available:${NC}"
echo -e "      Home     → http://localhost:${PORT}/frontend/index.html"
echo -e "      Report   → http://localhost:${PORT}/frontend/pages/report.html"
echo -e "      Outages  → http://localhost:${PORT}/frontend/pages/outages.html"
echo -e "      Predict  → http://localhost:${PORT}/frontend/pages/predict.html"
echo -e "      Admin    → http://localhost:${PORT}/frontend/pages/admin.html"
echo ""
echo -e "  ${YELLOW}Press Ctrl+C to stop the server${NC}"
echo ""

# Check if port is already in use
if lsof -Pi :$PORT -sTCP:LISTEN -t >/dev/null 2>&1; then
    echo -e "${YELLOW}  ⚠️  Port ${PORT} is in use. Trying ${PORT}1...${NC}"
    PORT=$((PORT + 1))
fi

python3 -m http.server $PORT 2>&1
