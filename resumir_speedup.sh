#!/bin/bash

CSV_ENTRADA=$1
if [ -z "$CSV_ENTRADA" ] || [ ! -f "$CSV_ENTRADA" ]; then
    echo "Uso: $0 <resultados_speedup_N....csv>"
    exit 1
fi

awk -F',' '
NR > 1 && $6 != "" {
    prog = $1;
    p = $3 + 0;
    key = prog "," p;
    count[key]++;
    vals[key, count[key]] = $6 + 0;
    pares[key] = $7;
    bloco[key] = $4;
}
END {
    for (k in count) {
        n = count[k];
        for (i = 1; i <= n; i++) {
            for (j = i + 1; j <= n; j++) {
                if (vals[k, i] > vals[k, j]) {
                    tmp = vals[k, i];
                    vals[k, i] = vals[k, j];
                    vals[k, j] = tmp;
                }
            }
        }
        med[k] = vals[k, int((n + 1) / 2)];
    }

    t_seq = med["seq,1"];
    if (t_seq <= 0) {
        print "Erro: mediana sequencial nao encontrada." > "/dev/stderr";
        exit 1;
    }

    printf "%-10s %-4s %-8s %-8s %-11s %-11s %-11s %-11s %-9s %-11s %-10s\n",
           "Versao", "P", "B", "Pares", "Exec1(s)", "Exec2(s)", "Exec3(s)", "Mediana(s)", "Speedup", "Efic.(frac)", "Efic.(%)";
    print "------------------------------------------------------------------------------------------------------------------------";

    split("seq ciclico mestre", progs, " ");
    split("1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16", ps, " ");

    for (pi = 1; pi <= 3; pi++) {
        pr = progs[pi];
        for (qi = 1; qi <= 16; qi++) {
            p = ps[qi];
            k = pr "," p;
            if (k in med) {
                spd = t_seq / med[k];
                efic_frac = spd / p;
                efic_pct = efic_frac * 100.0;
                printf "%-10s %-4d %-8s %-8s %-11.6f %-11.6f %-11.6f %-11.6f %-9.3f %-11.4f %-9.2f%%\n",
                       pr, p, bloco[k], pares[k], vals[k,1], vals[k,2], vals[k,3], med[k], spd, efic_frac, efic_pct;
            }
        }
    }
}' "$CSV_ENTRADA"
