// SEED: 0
// SCORE: 44151
// 1001000100011010001000100100111111111000110010000011000101111111110001100000100011000110001000110000

#include <bits/stdc++.h>
using namespace std;
using  ll = long long;

/* ---------- RNG (xoshiro256++) ---------- */
struct RNG {
    uint64_t s[4];
    static inline uint64_t rotl(uint64_t x,int k){ return (x<<k)|(x>>(64-k)); }
    explicit RNG(uint64_t seed=88172645463325252ULL){
        for(int i=0;i<4;i++) s[i]=seed+=0x9E3779B97F4A7C15ULL;
    }
    uint64_t next(){
        uint64_t res = rotl(s[1]*5,7)*9;
        uint64_t t = s[1]<<17;
        s[2]^=s[0]; s[3]^=s[1]; s[1]^=s[2]; s[0]^=s[3]; s[2]^=t; s[3]=rotl(s[3],45);
        return res;
    }
    int randint(int l,int r){ return int(next()%(r-l+1))+l; }
    double real01(){ return (next()>>11)*0x1p-53; }
};

constexpr int SCALE = 1<<10;

/* ---------- global ---------- */
int N,M;
vector<int> D,X,Y;
vector<vector<uint8_t>> windows;
vector<vector<pair<int,int>>> belongs;
vector<vector<int16_t>> tbl;

/* ---------- build smooth table ---------- */
void build_table(double sigma){
    for(int i=0;i<M;i++){
        int len=D[i];
        double inv2s2 = (sigma==0)?0:1.0/(2*sigma*sigma);
        for(int w=0;w<=len;++w){
            int v = (sigma==0 ? (w==X[i])*SCALE
                              : int(std::round(std::exp(-(w-X[i])*(w-X[i])*inv2s2)*SCALE)));
            tbl[i][w]=int16_t(v);
        }
    }
}

/* ---------- recompute score ---------- */
long long recompute(){
    long long s=0;
    for(int i=0;i<M;i++){
        for(size_t s0=0; s0<windows[i].size(); ++s0)
            s += 1LL * Y[i] * tbl[i][ windows[i][s0] ];
    }
    return s;
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if(!(cin>>N>>M)) return 0;
    D.resize(M); X.resize(M); Y.resize(M);
    for(int i=0;i<M;i++) cin>>D[i]>>X[i]>>Y[i];

    /* ---------- pre‑compute windows & belongs ---------- */
    windows.resize(M);
    belongs.assign(N,{});
    for(int i=0;i<M;i++){
        int len=D[i];
        windows[i].assign(N-len+1,0);
        for(int s=0;s<=N-len;++s)
            for(int k=0;k<len;++k)
                belongs[s+k].push_back({i,s});
    }

    /* ---------- initial greedy solution ---------- */
    string S(N,'0');
    for(int pos=0; pos<N; ++pos){
        long long g1=0,g0=0;
        for(auto [i,s0]: belongs[pos]){
            int w=windows[i][s0];
            if(w==X[i]-1) g1+=Y[i];
            if(w==X[i])   g0+=Y[i];
        }
        if(g1>=g0){
            S[pos]='1';
            for(auto [i,s0]: belongs[pos]) ++windows[i][s0];
        }
    }

    /* ---------- smooth table init ---------- */
    tbl.resize(M);
    for(int i=0;i<M;i++) tbl[i].resize(D[i]+1);

    const double sigma_vals[] = {4,3,2,1,0.5,0};
    const int PHASES = sizeof(sigma_vals)/sizeof(sigma_vals[0]);
    const int IT_TOTAL = 4000000;
    const int IT_PER  = IT_TOTAL/PHASES;

    build_table(sigma_vals[0]);
    long long score = recompute();

    RNG rng(chrono::high_resolution_clock::now().time_since_epoch().count());
    const double T0=1000.0, T_end=10.0;
    double T=T0, decay=pow(T_end/T0,1.0/IT_TOTAL);

    /* ---------- stderr dumps ---------- */
    const int TARGET_DUMP = 200;
    const int DUMP_INT = max(1, IT_TOTAL / TARGET_DUMP);
    int dump_cnt = 0;

    /* ---------- 1 点反転 Δ ---------- */
    auto flip_delta = [&](int pos,bool commit)->long long{
        bool to1 = (S[pos]=='0');
        long long d=0;
        for(auto [i,s0]: belongs[pos]){
            uint8_t &w=windows[i][s0];
            int v_old = tbl[i][w];
            int w_new = w + (to1?1:-1);
            int v_new = tbl[i][w_new];
            d += 1LL*Y[i]*(v_new - v_old);
            if(commit) w = uint8_t(w_new);
        }
        if(commit) S[pos]^=1;
        return d;
    };

    /* ---------- 2 点 SWAP Δ (正確) ---------- */
    auto swap_delta = [&](int a,int b,bool commit)->long long{
        if(a==b || S[a]==S[b]) return 0;            // 同じ位置または同一値なら効果なし
        long long d=0;
        if(!commit){
            long long d1 = flip_delta(a,true);      // a を一時反転
            long long d2 = flip_delta(b,false);     // その状態で b を評価
            flip_delta(a,true);                     // a を元に戻す
            d = d1 + d2;
        }else{
            d += flip_delta(a,true);                // 本採用：両方反転
            d += flip_delta(b,true);
        }
        return d;
    };

    /* ---------- Simulated Annealing ---------- */
    auto t_start = chrono::high_resolution_clock::now();
    int phase=0;
    for(int it=0; it<IT_TOTAL; ++it){
        /* phase 切替え */
        if(it && it%IT_PER==0 && phase+1<PHASES){
            ++phase;
            build_table(sigma_vals[phase]);
            score = recompute();
        }
        /* 時間切れ保険 (8.0s) */
        if(it%4000==0){
            auto now=chrono::high_resolution_clock::now();
            if(chrono::duration_cast<chrono::milliseconds>(now-t_start).count()>7800) break;
        }
        /* デバッグ出力 */
        if(it%DUMP_INT==0 && dump_cnt<TARGET_DUMP){
            cerr<<S<<"\n";
            ++dump_cnt;
        }

        /* --------------- 近傍操作選択 --------------- */
        if(rng.randint(0,1)==0){                              // ---- 50 % : FLIP ----
            int pos=rng.randint(0,N-1);
            long long d=flip_delta(pos,false);
            if(d>=0 || rng.real01()<exp(d/T)){
                flip_delta(pos,true);
                score+=d;
            }
        }else{                                                // ---- 50 % : SWAP ----
            int a=rng.randint(0,N-1);
            int b=rng.randint(0,N-2); if(b>=a) ++b;           // a≠b を保証
            long long d=swap_delta(a,b,false);
            if(d>=0 || rng.real01()<exp(d/T)){
                swap_delta(a,b,true);
                score+=d;
            }
        }
        T*=decay;
    }

    /* ---------- hill‑climb on true score ---------- */
    if(sigma_vals[PHASES-1]!=0){
        build_table(0);
        score=recompute();
    }
    bool improved=true;
    while(improved){
        improved=false;
        for(int pos=0; pos<N; ++pos){
            long long d=flip_delta(pos,false);
            if(d>0){
                flip_delta(pos,true);
                score+=d;
                improved=true;
            }
        }
    }

    /* stderr 200 行保証 */
    while(dump_cnt<TARGET_DUMP){
        cerr<<S<<"\n";
        ++dump_cnt;
    }

    cout<<S<<"\n";
    return 0;
}
