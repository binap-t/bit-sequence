// SEED: 0
// SCORE: 42901
// 1000110000100011000110000100011001111111110000100011000011000100011000110110001100010100010001100011

#include <bits/stdc++.h>
#define rep(i,n) for(int i=0;i<n;++i)
using namespace std;
typedef long long ll;

int N, M;
vector<int> D, X, Y;

int calc_score(const vector<int>& ANS) {
    vector<int> cum(N + 1, 0);
    rep(i, N) cum[i + 1] = cum[i] + ANS[i];

    int score = 0;
    rep(i, M) {
        for (int start = 0; start + D[i] <= N; ++start) {
            int cnt = cum[start + D[i]] - cum[start];
            if (cnt == X[i]) score += Y[i];
        }
    }
    return score;
}

pair<vector<int>, vector<int>> crossover(const vector<int>& a, const vector<int>& b, mt19937& rng) {
    uniform_int_distribution<int> dist(1, N - 1);
    int cut = dist(rng);
    vector<int> c1 = a, c2 = b;
    for (int i = cut; i < N; ++i) swap(c1[i], c2[i]);
    return {c1, c2};
}

void mutate(vector<int>& a, double mutation_rate, mt19937& rng) {
    uniform_real_distribution<> prob(0.0, 1.0);
    rep(i, N) if (prob(rng) < mutation_rate) a[i] ^= 1;
}

vector<int> tournament_select(const vector<pair<int, vector<int>>>& pop, int k, mt19937& rng) {
    uniform_int_distribution<int> dist(0, pop.size() - 1);
    int best_score = -1;
    vector<int> best;
    rep(i, k) {
        const auto& cand = pop[dist(rng)];
        if (cand.first > best_score) {
            best_score = cand.first;
            best = cand.second;
        }
    }
    return best;
}

vector<int> genetic_algorithm(int generations = 8000, int population_size = 200, double mutation_rate = 0.01) {
    mt19937 rng(0);
    uniform_int_distribution<int> bin(0, 1);
    vector<pair<int, vector<int>>> population;

    rep(_, population_size) {
        vector<int> individual(N);
        rep(i, N) individual[i] = bin(rng);
        int score = calc_score(individual);
        population.emplace_back(score, individual);
    }

    pair<int, vector<int>> best = *max_element(population.begin(), population.end());

    const int log_interval = max(1, generations / 200);

    rep(g, generations) {
        vector<pair<int, vector<int>>> new_population;
        while ((int)new_population.size() < population_size) {
            auto p1 = tournament_select(population, 3, rng);
            auto p2 = tournament_select(population, 3, rng);
            auto [c1, c2] = crossover(p1, p2, rng);
            mutate(c1, mutation_rate, rng);
            mutate(c2, mutation_rate, rng);
            new_population.emplace_back(calc_score(c1), c1);
            new_population.emplace_back(calc_score(c2), c2);
        }

        sort(new_population.rbegin(), new_population.rend());
        population.assign(new_population.begin(), new_population.begin() + population_size);

        if (population[0].first > best.first) best = population[0];

        // 生の解を出力
        if (g % log_interval == 0 || g == generations - 1) {
            for (int x : best.second) cerr << x;
            cerr << '\n';
        }
    }

    return best.second;
}

int main() {
    cin >> N >> M;
    D.resize(M), X.resize(M), Y.resize(M);
    rep(i, M) cin >> D[i] >> X[i] >> Y[i];

    vector<int> ANS = genetic_algorithm();

    for (int x : ANS) cout << x;
    cout << "\n";

    cout << calc_score(ANS) << "\n";
    return 0;
}
