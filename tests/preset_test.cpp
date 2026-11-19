#include "../Source/Engine.h"
#include "../Source/Presets.h"
#include <cstdio>
#include <cstring>
using namespace juno;
int main()
{
    int errors = 0; const ParamDef* d = paramDefs();
    for (const auto& pr : presets()) {
        float p[P::COUNT]; for (int i = 0; i < P::COUNT; ++i) p[i] = d[i].def;
        for (auto& kv : pr.v) {
            int idx = -1; for (int i = 0; i < P::COUNT; ++i) if (!std::strcmp(d[i].id, kv.first)) idx = i;
            if (idx < 0) { printf("UNKNOWN PARAM '%s' in %s\n", kv.first, pr.name); ++errors; continue; }
            if (kv.second < d[idx].mn || kv.second > d[idx].mx) { printf("OUT OF RANGE %s in %s\n", kv.first, pr.name); ++errors; }
            p[idx] = kv.second;
        }
        Engine e; e.prepare(48000); e.setParams(p);
        for (int n : { 48, 55, 60, 64 }) e.noteOn(n, 0.8f);
        std::vector<float> L(48000 * 2), R(48000 * 2); e.process(L.data(), R.data(), (int)L.size());
        float pk = 0; bool bad = false; for (size_t i = 0; i < L.size(); ++i) { pk = std::max(pk, std::max(std::fabs(L[i]), std::fabs(R[i]))); bad |= !std::isfinite(L[i]) || !std::isfinite(R[i]); }
        printf("%-22s peak %.2f%s%s\n", pr.name, pk, pk > 0.95f ? "  <-- HOT" : "", pk < 0.03f ? "  <-- QUIET" : "");
        if (bad) { printf("  NON-FINITE\n"); ++errors; }
    }
    printf("%zu presets, %d errors\n", presets().size(), errors);
    return errors != 0;
}
