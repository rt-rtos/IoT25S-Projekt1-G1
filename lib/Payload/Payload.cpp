#include "Payload.h"
#include <stdio.h>
#include <stdarg.h>
#include <math.h>

namespace {

// Appends formatted text, keeps counting past the end like snprintf.
struct Out {
    char*  p;
    size_t left;
    int    total = 0;

    void add(const char* fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        int n = vsnprintf(p, left, fmt, ap);
        va_end(ap);
        if (n < 0) return;
        total += n;
        size_t used = (size_t)n < left ? (size_t)n : (left ? left - 1 : 0);
        p += used;
        left -= used;
    }

    // "24.1" / "-3.5" / "null", one decimal, no float printf.
    void value(const Reading& r) {
        if (!r.valid || isnan(r.value)) { add("null"); return; }
        long tenths = lroundf(r.value * 10.0f);
        add("%s%ld.%ld", tenths < 0 ? "-" : "", labs(tenths) / 10, labs(tenths) % 10);
    }
};

struct Channel {
    const char*    key;
    const Reading& r;
};

} // namespace

int buildTelemetryJson(char* buf, size_t len, const Snapshot& s, const SourceInfo& src) {
    const Channel ch[] = {
        { "t_in",    s.tIn    },
        { "rh_in",   s.rhIn   },
        { "t_out",   s.tOut   },
        { "t_water", s.tWater },
    };
    Out o{ buf, len };

    o.add(R"({"seq":%lu,"uptime_s":%lu)", (unsigned long)s.seq, (unsigned long)(s.uptimeMs / 1000));
    for (const Channel& c : ch) {
        o.add(R"(,"%s":)", c.key);
        o.value(c.r);
    }
    o.add(R"(,"faults":{)");
    for (size_t i = 0; i < 4; ++i) {
        o.add(R"(%s"%s":%u)", i ? "," : "", ch[i].key, ch[i].r.fault);
    }
    o.add(R"(},"src":{"sht40":"%s","ds18b20":"%s","ntc":"%s"}})", src.sht40, src.ds18b20, src.ntc);
    return o.total;
}
