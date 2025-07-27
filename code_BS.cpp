// SEED: 0
// SCORE: 42582
// 1000100011000110001000110001101000101000111000100010100011100010100011100010001010001110001010001110

#include <bits/stdc++.h>
#define rep(i,n) for(int i=0;i<(int)(n);++i)
using namespace std;
typedef long long ll;

int N, M;
vector<int> D, X, Y;

// スコア計算関数
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

// ビームサーチ（左から1bitずつ確定）
vector<int> beam_search(int beam_width = 100) {
    using State = pair<int, vector<int>>; // (score, partial solution)

    vector<State> beam = {{0, vector<int>()}};  // 空の初期状態

    for (int pos = 0; pos < N; ++pos) {
        vector<State> next_beam;

        for (const auto& [score, partial] : beam) {
            // 0 を追加
            vector<int> cand0 = partial;
            cand0.push_back(0);
            vector<int> padded0 = cand0;
            padded0.resize(N, 0);
            int score0 = calc_score(padded0);
            cand0.resize(pos + 1);
            next_beam.emplace_back(score0, cand0);

            // 1 を追加
            vector<int> cand1 = partial;
            cand1.push_back(1);
            vector<int> padded1 = cand1;
            padded1.resize(N, 0);
            int score1 = calc_score(padded1);
            cand1.resize(pos + 1);
            next_beam.emplace_back(score1, cand1);
        }

        // スコア降順でソートして beam_width に絞る
        sort(next_beam.rbegin(), next_beam.rend());
        if ((int)next_beam.size() > beam_width)
            next_beam.resize(beam_width);
        beam = move(next_beam);

        // 最良解（右側0埋め）をcerrに出力
        const auto& best_partial = beam[0].second;
        vector<int> padded = best_partial;
        padded.resize(N, 0);
        for (int x : padded) cerr << x;
        cerr << '\n';
    }

    return max_element(beam.begin(), beam.end())->second;
}

int main() {
    cin >> N >> M;
    D.resize(M), X.resize(M), Y.resize(M);
    rep(i, M) cin >> D[i] >> X[i] >> Y[i];

    vector<int> ANS = beam_search(100);

    for (int x : ANS) cout << x;
    cout << "\n";

    cout << calc_score(ANS) << "\n";
    return 0;
}
