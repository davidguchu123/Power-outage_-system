# ============================================================
# Makefile — PowerWatch Kenya C++ Backend
# ============================================================
# Usage:
#   make          → Build the CGI binary
#   make clean    → Remove compiled binary
#   make run      → Start the local web server (Python)
#   make demo     → Run backend tests in terminal
# ============================================================

CXX      = g++
CXXFLAGS = -std=c++11 -O2 -Wall -Wno-stringop-truncation
TARGET   = outage_api
SRC      = main.cpp

# Default: build
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)
	@echo "✅  Built: backend/$(TARGET)"

# For CGI deployment: rename with .cgi extension
cgi: $(TARGET)
	cp $(TARGET) $(TARGET).cgi
	chmod 755 $(TARGET).cgi
	@echo "✅  CGI binary ready: backend/$(TARGET).cgi"

# Run a quick terminal demo of the C++ backend
demo: $(TARGET)
	@echo "=== LIST (first 3) ==="
	@QUERY_STRING="action=list" REQUEST_METHOD=GET ./$(TARGET) | python3 -c \
	  "import sys; body=sys.stdin.read().split('\r\n\r\n',1)[-1]; \
	   import json; d=json.loads(body); [print(r) for r in d[:3]]" 2>/dev/null || true
	@echo ""
	@echo "=== STATS ==="
	@QUERY_STRING="action=stats" REQUEST_METHOD=GET ./$(TARGET) | python3 -c \
	  "import sys; body=sys.stdin.read().split('\r\n\r\n',1)[-1]; print(body)"
	@echo ""
	@echo "=== PREDICT Nairobi CBD ==="
	@QUERY_STRING="action=predict&location=Nairobi+CBD" REQUEST_METHOD=GET ./$(TARGET) | python3 -c \
	  "import sys; body=sys.stdin.read().split('\r\n\r\n',1)[-1]; print(body)"

# Start the web server from the project root
run:
	@echo "Starting PowerWatch Kenya on http://localhost:8080"
	@echo "Open your browser at: http://localhost:8080/frontend/index.html"
	@cd .. && python3 -m http.server 8080

clean:
	rm -f $(TARGET) $(TARGET).cgi
	@echo "Cleaned."

.PHONY: all cgi demo run clean
