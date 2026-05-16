/*
 * ============================================================
 * Power Outage Prediction & Reporting System
 * Backend: C++ with OOP Principles
 * Author: University Project - Kenya Power Outage System
 * ============================================================
 *
 * OOP PRINCIPLES USED:
 * [ENCAPSULATION]  - Private/protected members with getters/setters
 * [INHERITANCE]    - User → ResidentUser, AdminUser
 * [POLYMORPHISM]   - Virtual functions (predictOutage, displayReport)
 * [ABSTRACTION]    - Abstract PredictionSystem class
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <cmath>

using namespace std;

// ============================================================
// CONSTANTS
// ============================================================
const int MAX_REPORTS  = 200;
const int MAX_LOCATION = 100;
const int MAX_NAME     = 100;

// ============================================================
// OutageReport CLASS
// [ENCAPSULATION] - All attributes are private; accessed via
//                   getters/setters
// ============================================================
class OutageReport {
private:
    // Private attributes — ENCAPSULATION
    char   location[MAX_LOCATION];
    char   reportedBy[MAX_NAME];
    char   startTime[30];
    double duration;          // hours
    char   status[20];        // "active" or "resolved"
    int    reportId;

public:
    // Constructor
    OutageReport() {
        strcpy(location,   "Unknown");
        strcpy(reportedBy, "Unknown");
        strcpy(startTime,  "N/A");
        duration = 0.0;
        strcpy(status, "active");
        reportId = 0;
    }

    // ---- Setters (ENCAPSULATION) ----
    void setLocation(const char* loc)   { strncpy(location,   loc,  MAX_LOCATION - 1); }
    void setReportedBy(const char* name){ strncpy(reportedBy, name, MAX_NAME - 1);     }
    void setStartTime(const char* time) { strncpy(startTime,  time, 29);               }
    void setDuration(double d)          { duration = d;                                 }
    void setStatus(const char* s)       { strncpy(status,     s,    19);               }
    void setReportId(int id)            { reportId = id;                                }

    // ---- Getters (ENCAPSULATION) ----
    const char* getLocation()   const { return location;   }
    const char* getReportedBy() const { return reportedBy; }
    const char* getStartTime()  const { return startTime;  }
    double      getDuration()   const { return duration;   }
    const char* getStatus()     const { return status;     }
    int         getReportId()   const { return reportId;   }

    // [POLYMORPHISM] - virtual method, can be overridden
    virtual void displayReport() const {
        cout << "Report #"    << reportId
             << " | Location: " << location
             << " | By: "       << reportedBy
             << " | Start: "    << startTime
             << " | Duration: " << duration << "h"
             << " | Status: "   << status   << endl;
    }

    // Serialize report to a single CSV line for file storage
    void serialize(char* buffer, int bufSize) const {
        snprintf(buffer, bufSize, "%d|%s|%s|%s|%.2f|%s\n",
                 reportId, location, reportedBy, startTime, duration, status);
    }

    // Deserialize from a CSV line
    void deserialize(const char* line) {
        char buf[512];
        strncpy(buf, line, 511);
        char* token = strtok(buf, "|");
        if (token) reportId = atoi(token);

        token = strtok(NULL, "|");
        if (token) strncpy(location, token, MAX_LOCATION - 1);

        token = strtok(NULL, "|");
        if (token) strncpy(reportedBy, token, MAX_NAME - 1);

        token = strtok(NULL, "|");
        if (token) strncpy(startTime, token, 29);

        token = strtok(NULL, "|");
        if (token) duration = atof(token);

        token = strtok(NULL, "|");
        if (token) {
            strncpy(status, token, 19);
            // Strip newline
            int len = strlen(status);
            if (len > 0 && (status[len-1] == '\n' || status[len-1] == '\r'))
                status[len-1] = '\0';
        }
    }
};

// ============================================================
// ABSTRACT PredictionSystem CLASS
// [ABSTRACTION] - Pure virtual function makes this abstract;
//                 cannot instantiate directly
// ============================================================
class PredictionSystem {
protected:
    char targetLocation[MAX_LOCATION];

public:
    PredictionSystem() { strcpy(targetLocation, "All"); }

    void setTargetLocation(const char* loc) {
        strncpy(targetLocation, loc, MAX_LOCATION - 1);
    }

    // [ABSTRACTION] Pure virtual — must be implemented by derived classes
    virtual void predictOutage(OutageReport* reports, int count) = 0;

    // Virtual destructor — good OOP practice
    virtual ~PredictionSystem() {}
};

// ============================================================
// SimplePrediction CLASS (Derived from PredictionSystem)
// [INHERITANCE]   - Inherits from PredictionSystem
// [POLYMORPHISM]  - Overrides pure virtual predictOutage()
// ============================================================
class SimplePrediction : public PredictionSystem {
private:
    // Private helper: count outages in target location
    int countOutagesInLocation(OutageReport* reports, int count) const {
        int c = 0;
        for (int i = 0; i < count; i++) {
            if (strcmp(reports[i].getLocation(), targetLocation) == 0)
                c++;
        }
        return c;
    }

    // Private helper: compute average duration for target location
    double averageDuration(OutageReport* reports, int count) const {
        double total = 0.0;
        int    num   = 0;
        for (int i = 0; i < count; i++) {
            if (strcmp(reports[i].getLocation(), targetLocation) == 0) {
                total += reports[i].getDuration();
                num++;
            }
        }
        return (num > 0) ? (total / num) : 0.0;
    }

public:
    // [POLYMORPHISM] — Implements the abstract method
    void predictOutage(OutageReport* reports, int count) override {
        int    freq   = countOutagesInLocation(reports, count);
        double avgDur = averageDuration(reports, count);

        cerr << "=== Prediction for: " << targetLocation << " ===" << endl;
        cerr << "Total past outages : " << freq << endl;
        cerr << "Average duration   : " << avgDur << " hours" << endl;

        if (freq >= 5) {
            cerr << "Risk Level: HIGH - Frequent outages detected!" << endl;
            cerr << "Alert: Possible outage in your area"           << endl;
        } else if (freq >= 2) {
            cerr << "Risk Level: MEDIUM - Occasional outages."      << endl;
        } else {
            cerr << "Risk Level: LOW - Rare outages."               << endl;
        }
    }

    // Returns JSON-formatted prediction for the CGI/API layer
    void predictOutageJSON(OutageReport* reports, int count,
                            char* outBuf, int outSize) const {
        int    freq   = countOutagesInLocation(reports, count);
        double avgDur = averageDuration(reports, count);

        const char* riskLevel;
        const char* message;
        const char* smsAlert;

        if (freq >= 5) {
            riskLevel = "HIGH";
            message   = "Frequent outages detected. High probability of another soon.";
            smsAlert  = "ALERT: Possible outage in your area. Prepare backup power.";
        } else if (freq >= 2) {
            riskLevel = "MEDIUM";
            message   = "Occasional outages on record. Stay alert.";
            smsAlert  = "Notice: Moderate outage risk in your area.";
        } else if (freq == 1) {
            riskLevel = "LOW";
            message   = "Only one prior outage recorded. Low risk.";
            smsAlert  = "Info: Low outage risk in your area.";
        } else {
            riskLevel = "NONE";
            message   = "No outages recorded for this location.";
            smsAlert  = "No outage risk detected.";
        }

        snprintf(outBuf, outSize,
            "{"
            "\"location\":\"%s\","
            "\"frequency\":%d,"
            "\"avgDuration\":%.2f,"
            "\"riskLevel\":\"%s\","
            "\"message\":\"%s\","
            "\"smsAlert\":\"%s\""
            "}",
            targetLocation, freq, avgDur, riskLevel, message, smsAlert);
    }
};

// ============================================================
// USER BASE CLASS
// [ENCAPSULATION] - name and location are private
// [INHERITANCE]   - Base for ResidentUser and AdminUser
// ============================================================
class User {
private:
    // Private attributes — ENCAPSULATION
    char name[MAX_NAME];
    char location[MAX_LOCATION];
    char userType[20];

protected:
    // Protected so derived classes can read (but not bypass setters)
    const char* getName()     const { return name;     }
    const char* getLocation() const { return location; }

public:
    User() {
        strcpy(name,     "Anonymous");
        strcpy(location, "Unknown");
        strcpy(userType, "user");
    }

    // Setters — ENCAPSULATION
    void setName(const char* n)     { strncpy(name,     n, MAX_NAME     - 1); }
    void setLocation(const char* l) { strncpy(location, l, MAX_LOCATION - 1); }
    void setUserType(const char* t) { strncpy(userType, t, 19);               }
    const char* getUserType() const { return userType; }

    // [POLYMORPHISM] - Virtual method overridden in derived classes
    virtual void reportOutage(OutageReport* reports, int& count) {
        cerr << "[User] reportOutage() called" << endl;
    }

    virtual ~User() {}
};

// ============================================================
// ResidentUser CLASS (Derived from User)
// [INHERITANCE]  - "is-a" User
// [POLYMORPHISM] - Overrides reportOutage()
// ============================================================
class ResidentUser : public User {
public:
    ResidentUser() { setUserType("resident"); }

    // [POLYMORPHISM] — Overrides base class virtual method
    void reportOutage(OutageReport* reports, int& count) override {
        if (count >= MAX_REPORTS) {
            cerr << "Report limit reached." << endl;
            return;
        }

        OutageReport& r = reports[count];
        r.setReportId(count + 1);
        r.setLocation(getLocation());
        r.setReportedBy(getName());

        // Capture current time as startTime
        time_t now = time(NULL);
        char   buf[30];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", localtime(&now));
        r.setStartTime(buf);

        r.setDuration(0.0);   // Active outage — duration unknown yet
        r.setStatus("active");

        count++;
        cerr << "Resident " << getName()
             << " reported outage in " << getLocation() << endl;
    }
};

// ============================================================
// AdminUser CLASS (Derived from User)
// [INHERITANCE]  - "is-a" User
// [POLYMORPHISM] - Overrides reportOutage(); adds admin methods
// ============================================================
class AdminUser : public User {
public:
    AdminUser() { setUserType("admin"); }

    // [POLYMORPHISM] — Overrides reportOutage() with admin behavior
    void reportOutage(OutageReport* reports, int& count) override {
        cerr << "[Admin] Logging system-detected outage for " << getLocation() << endl;
        ResidentUser tmp;
        tmp.setName(getName());
        tmp.setLocation(getLocation());
        tmp.reportOutage(reports, count);
        // Admin also marks status as confirmed
        if (count > 0)
            reports[count - 1].setStatus("confirmed");
    }

    // Admin-only: view all reports
    void viewAllReports(OutageReport* reports, int count) const {
        cout << "=== ALL OUTAGE REPORTS ===" << endl;
        for (int i = 0; i < count; i++)
            reports[i].displayReport();
    }

    // Admin-only: mark a report resolved
    void resolveReport(OutageReport* reports, int count, int id, double duration) {
        for (int i = 0; i < count; i++) {
            if (reports[i].getReportId() == id) {
                reports[i].setStatus("resolved");
                reports[i].setDuration(duration);
                cerr << "Report #" << id << " resolved." << endl;
                return;
            }
        }
        cerr << "Report #" << id << " not found." << endl;
    }
};

// ============================================================
// ReportStorage — handles file-based persistence
// [ENCAPSULATION] - Internal file path is private
// ============================================================
class ReportStorage {
private:
    const char* filePath;

public:
    ReportStorage(const char* path) : filePath(path) {}

    // Save all reports to file
    bool saveAll(OutageReport* reports, int count) const {
        ofstream f(filePath);
        if (!f.is_open()) return false;
        char buf[512];
        for (int i = 0; i < count; i++) {
            reports[i].serialize(buf, 512);
            f << buf;
        }
        f.close();
        return true;
    }

    // Load reports from file; returns count loaded
    int loadAll(OutageReport* reports, int maxCount) const {
        ifstream f(filePath);
        if (!f.is_open()) return 0;
        int count = 0;
        char line[512];
        while (f.getline(line, 512) && count < maxCount) {
            if (strlen(line) > 3) {
                reports[count].deserialize(line);
                count++;
            }
        }
        f.close();
        return count;
    }
};

// ============================================================
// CGI / API LAYER — outputs JSON for frontend fetch()
// Reads ?action= query string and routes to the right handler
// ============================================================

// Utility: URL-decode a query string value
void urlDecode(const char* src, char* dst, int dstLen) {
    int j = 0;
    for (int i = 0; src[i] && j < dstLen - 1; i++) {
        if (src[i] == '+') {
            dst[j++] = ' ';
        } else if (src[i] == '%' && src[i+1] && src[i+2]) {
            char hex[3] = { src[i+1], src[i+2], 0 };
            dst[j++] = (char)strtol(hex, NULL, 16);
            i += 2;
        } else {
            dst[j++] = src[i];
        }
    }
    dst[j] = '\0';
}

// Extract a named parameter from query string
void getParam(const char* query, const char* key,
              char* value, int valueLen) {
    value[0] = '\0';
    if (!query) return;

    char buf[2048];
    strncpy(buf, query, 2047);

    char keyEq[128];
    snprintf(keyEq, sizeof(keyEq), "%s=", key);

    char* pos = strstr(buf, keyEq);
    if (!pos) return;

    pos += strlen(keyEq);
    char raw[1024];
    int  i = 0;
    while (pos[i] && pos[i] != '&' && i < 1023) {
        raw[i] = pos[i];
        i++;
    }
    raw[i] = '\0';

    urlDecode(raw, value, valueLen);
}

// Output HTTP + JSON headers
void printJSONHeader() {
    cout << "Content-Type: application/json\r\n";
    cout << "Access-Control-Allow-Origin: *\r\n";
    cout << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
    cout << "Access-Control-Allow-Headers: Content-Type\r\n";
    cout << "\r\n";
}

// ============================================================
// MAIN — CGI entry point
// ============================================================
int main() {
    const char* dataFile = "../data/reports.dat";
    ReportStorage storage(dataFile);

    // Load existing reports
    static OutageReport reports[MAX_REPORTS];
    int reportCount = storage.loadAll(reports, MAX_REPORTS);

    // Read CGI query string (GET) or POST body
    const char* method   = getenv("REQUEST_METHOD");
    const char* queryStr = getenv("QUERY_STRING");
    char        postBody[2048] = "";

    if (method && strcmp(method, "POST") == 0) {
        const char* lenStr = getenv("CONTENT_LENGTH");
        if (lenStr) {
            int len = atoi(lenStr);
            if (len > 2047) len = 2047;
            cin.read(postBody, len);
            postBody[len] = '\0';
            queryStr = postBody;   // treat POST body same as query string
        }
    }

    // Parse action
    char action[64] = "list";
    getParam(queryStr, "action", action, 64);

    printJSONHeader();

    // --------------------------------------------------------
    // ACTION: list — return all reports as JSON
    // --------------------------------------------------------
    if (strcmp(action, "list") == 0) {
        cout << "[";
        for (int i = 0; i < reportCount; i++) {
            cout << "{"
                 << "\"id\":"       << reports[i].getReportId()  << ","
                 << "\"location\":\"" << reports[i].getLocation() << "\","
                 << "\"reportedBy\":\"" << reports[i].getReportedBy() << "\","
                 << "\"startTime\":\"" << reports[i].getStartTime() << "\","
                 << "\"duration\":" << reports[i].getDuration()   << ","
                 << "\"status\":\"" << reports[i].getStatus()     << "\""
                 << "}";
            if (i < reportCount - 1) cout << ",";
        }
        cout << "]";
    }

    // --------------------------------------------------------
    // ACTION: add — add a new outage report
    // --------------------------------------------------------
    else if (strcmp(action, "add") == 0) {
        char name[MAX_NAME]         = "Anonymous";
        char loc[MAX_LOCATION]      = "Unknown";
        char startTime[30]          = "";
        char userType[20]           = "resident";
        char durationStr[20]        = "0";

        getParam(queryStr, "name",      name,        MAX_NAME);
        getParam(queryStr, "location",  loc,         MAX_LOCATION);
        getParam(queryStr, "startTime", startTime,   30);
        getParam(queryStr, "userType",  userType,    20);
        getParam(queryStr, "duration",  durationStr, 20);

        if (strlen(startTime) == 0) {
            time_t now = time(NULL);
            strftime(startTime, sizeof(startTime),
                     "%Y-%m-%d %H:%M", localtime(&now));
        }

        // Use appropriate User subclass — POLYMORPHISM in action
        User* user = NULL;
        if (strcmp(userType, "admin") == 0) {
            user = new AdminUser();
        } else {
            user = new ResidentUser();
        }
        user->setName(name);
        user->setLocation(loc);

        // reportOutage() calls polymorphically
        user->reportOutage(reports, reportCount);

        // Patch startTime and duration from form params
        if (reportCount > 0) {
            reports[reportCount - 1].setStartTime(startTime);
            reports[reportCount - 1].setDuration(atof(durationStr));
        }

        delete user;

        storage.saveAll(reports, reportCount);

        cout << "{"
             << "\"success\":true,"
             << "\"message\":\"Report submitted successfully.\","
             << "\"totalReports\":" << reportCount
             << "}";
    }

    // --------------------------------------------------------
    // ACTION: predict — run SimplePrediction for a location
    // --------------------------------------------------------
    else if (strcmp(action, "predict") == 0) {
        char loc[MAX_LOCATION] = "All";
        getParam(queryStr, "location", loc, MAX_LOCATION);

        // [ABSTRACTION + POLYMORPHISM] — Using abstract base pointer
        PredictionSystem* predictor = new SimplePrediction();
        predictor->setTargetLocation(loc);

        char predBuf[1024];
        // Cast to SimplePrediction to call JSON method
        static_cast<SimplePrediction*>(predictor)
            ->predictOutageJSON(reports, reportCount, predBuf, 1024);

        cout << predBuf;
        delete predictor;
    }

    // --------------------------------------------------------
    // ACTION: resolve — mark a report resolved
    // --------------------------------------------------------
    else if (strcmp(action, "resolve") == 0) {
        char idStr[20]  = "0";
        char durStr[20] = "0";
        getParam(queryStr, "id",       idStr,  20);
        getParam(queryStr, "duration", durStr, 20);

        AdminUser admin;
        admin.setName("System");
        admin.setLocation("All");
        admin.resolveReport(reports, reportCount,
                            atoi(idStr), atof(durStr));
        storage.saveAll(reports, reportCount);

        cout << "{\"success\":true,\"message\":\"Report resolved.\"}";
    }

    // --------------------------------------------------------
    // ACTION: stats — summary statistics
    // --------------------------------------------------------
    else if (strcmp(action, "stats") == 0) {
        int    active   = 0, resolved = 0;
        double totalDur = 0.0;

        for (int i = 0; i < reportCount; i++) {
            if (strcmp(reports[i].getStatus(), "resolved") == 0) {
                resolved++;
                totalDur += reports[i].getDuration();
            } else {
                active++;
            }
        }
        double avgDur = (resolved > 0) ? (totalDur / resolved) : 0.0;

        cout << "{"
             << "\"totalReports\":"  << reportCount << ","
             << "\"activeOutages\":" << active      << ","
             << "\"resolvedCount\":" << resolved    << ","
             << "\"avgDuration\":"   << avgDur
             << "}";
    }

    // --------------------------------------------------------
    // Unknown action
    // --------------------------------------------------------
    else {
        cout << "{\"error\":\"Unknown action: " << action << "\"}";
    }

    return 0;
}
