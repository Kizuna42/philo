# 🍝 philo — 食事する哲学者問題を pthread で解く

![language](https://img.shields.io/badge/language-C-blue.svg)
![norm](https://img.shields.io/badge/42-norminette-success.svg)
![flags](https://img.shields.io/badge/build-Wall%20Wextra%20Werror%20pthread-brightgreen.svg)
![platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS-lightgrey.svg)

> **TL;DR (EN):** The classic Dining Philosophers problem solved with POSIX threads,
> written in C as a [42](https://42tokyo.jp/) project. Each philosopher is one
> `pthread`; each fork is a `pthread_mutex_t`; a dedicated monitor thread owns every
> termination decision so the philosopher threads carry no end-of-run logic. Three
> mutexes guard three separate concerns (stdout, the death flag, meal bookkeeping)
> instead of one global lock. **623 lines across 9 files, 29 functions, zero heap
> allocation** — no `malloc` appears anywhere, so there is no `free` to leak.
> Builds warning-free under `-Wall -Wextra -Werror`; all 9 files pass norminette.

42 の課題 **philosophers** の提出物です。哲学者 N 人が円卓に座り、左右のフォークを
両方取れたときだけ食事し、それ以外は睡眠と思考を繰り返します。誰か 1 人でも
`time_to_die` ミリ秒のあいだ食事できなければ死亡としてプログラムを終了する、という
要求を pthread と mutex だけで満たします。

提出対象は **`philo/`** 配下です。`philo_annotated/` は同じコードに日本語の解説コメントを
付けた学習用のコピーで、提出物ではありません（[後述](#philo_annotated-について)）。

## ビルド・実行

Makefile はリポジトリ直下ではなく `philo/` にあります。ターゲットは `all` / `clean` /
`fclean` / `re`。`CC = cc`、`CFLAGS = -Wall -Wextra -Werror -pthread`。外部ライブラリに
依存しないので libft 等を先にビルドする必要はありません。

```bash
cd philo && make
./philo number_of_philosophers time_to_die time_to_eat time_to_sleep [number_of_times_each_philosopher_must_eat]
```

時間の単位はすべてミリ秒。第 5 引数は省略可で、指定した場合は全員がその回数だけ食べ
終わった時点で正常終了します。省略時は誰かが死ぬまで走り続けます。`.gitignore` を
置いていないため `make` 後は `obj/` とバイナリが未追跡で残ります（`make fclean` で消えます）。

```console
$ ./philo 4 410 200 200 2      # 全員が2回食べ終わって自動終了（実際の出力から抜粋）
0 1 has taken a fork
0 1 has taken a fork
0 1 is eating
0 3 has taken a fork
0 3 has taken a fork
0 3 is eating
200 3 is sleeping
200 1 is sleeping
200 2 has taken a fork
200 2 has taken a fork
200 2 is eating
200 4 has taken a fork
200 4 has taken a fork
200 4 is eating
（中略）
600 2 is eating

$ ./philo 1 800 200 200        # フォークが1本しかないので必ず餓死する
0 1 has taken a fork
800 1 died

$ ./philo abc 800 200 200      # 引数エラーはすべて exit 1
Error: Invalid argument 'abc'
```

1・3 → 2・4 と奇数側と偶数側が交互に食事しています（理由は[後述](#デッドロック回避は資源階層ではなくタイミングのずらし)）。
タイムスタンプと食事の順序はスケジューラ次第で実行ごとに変わります。

## 実装のポイント

### ヒープを使わない

`main.c` は 29 行です。`t_philo philos[200]` と `pthread_mutex_t forks[200]` を main の
スタックに固定長で確保し、以降 `malloc` を一度も呼びません。`free` 漏れの経路が構造的に
存在しない代わりに、哲学者数がこの配列長に縛られます。`input.c` の「200 人まで」という
上限はこの設計の直接の帰結で、動的確保にすれば外せる割り切りです。

### 終了判定を監視スレッドに集約する

哲学者スレッドは終了条件を一切知りません。`philo_routine` は「死亡フラグが立つまで
eat → sleep → think を回す」だけで、判定は `monitor` スレッド 1 本が持ちます。

- `check_death_and_set_flag` — `meal_lock` の下で `get_current_time() - last_meal >= time_to_die` かつ `eating == 0` を全員について確認（1 周 O(n)）
- `check_if_all_ate` — 第 5 引数がある場合のみ、全員の `meals_eaten` が規定回数に達したかを確認（同じく O(n)。第 5 引数なしなら即 `return (0)` でスキップ）

どちらかが成立したら `dead_flag` を立てて監視ループを抜けます。`thread_create` は監視を
哲学者より先に生成し、join も監視 → 哲学者の順。監視が返った時点でフラグは立っているので、
各哲学者は進行中の 1 周を終えてループ条件で自然に抜け、全員が join できます（detach や
シグナルによる強制終了は不要）。監視ループは sleep を挟まずスピンで回すため、検知は速い
一方で CPU を消費します（`time_to_die = 310` に対し died 出力が 310〜322 ms、5 回実行）。

### mutex を保護対象ごとに分ける

共有 mutex を 1 本の巨大ロックにせず、`write_lock`（標準出力）、`dead_lock`（終了フラグ）、
`meal_lock`（`last_meal` と `meals_eaten`）の 3 本に分けています。フォークはこれとは別に
`forks[i]` の per-mutex 配列です。分けたことで、監視スレッドが `meal_lock` を握って全員を
走査している最中でも他のスレッドは出力を続けられます。

`t_philo` は共有データをポインタで持つため「スレッドローカルな状態 + 共有資源への参照」
という一貫した形になり、`philo_routine` へは `&philos[i]` を 1 つ渡すだけで済みます。

### 死亡メッセージだけログ抑止を迂回させる

課題は「死亡後に他のログを出してはいけない」と「死亡は必ず報告する」の両方を要求します。
`print_message` は通常のメッセージなら `dead_lock` を取って `dead_flag == 0` のときだけ
`printf` しますが、`"died"` だけは `ft_strcmp` で判定してフラグのチェックを飛ばします。
`write_lock` は全経路で取るので行が混ざることはありません。

### 哲学者 1 人の自己デッドロックを潰す

`assign_forks` は哲学者 i に `l_fork = forks[i]`、`r_fork = forks[i - 1]`（i == 0 のみ
`forks[n - 1]`）を割り当てます。N = 1 では両者が同じ `forks[0]` を指すため、素直に両方
ロックすると自分で自分を待ってハングします。`take_forks` で明示的に分岐しています。

```c
	pthread_mutex_lock(philo->r_fork);
	print_message("has taken a fork", philo, philo->id);
	if (philo->num_of_philos == 1)
	{
		ft_usleep(philo->time_to_die);
		pthread_mutex_unlock(philo->r_fork);
		return ;
	}
	pthread_mutex_lock(philo->l_fork);
```

`eat()` 側にも `num_of_philos == 1` の早期 return があり、フォーク 1 本を取って
`time_to_die` 待って死ぬ、という正しい振る舞いになります。

### デッドロック回避は資源階層ではなくタイミングのずらし

ここは正直に書きます。`take_forks` は全員が `r_fork` → `l_fork` の順にロックします。
これはリング上の相対位置としては対称な順序で、**循環待ちの条件そのものは排除できて
いません**。添字 1..n-1 の哲学者は添字の小さいフォークを先に取りますが、添字 0 の哲学者（ログ上の
ID は 1）だけは `forks[n-1]` を先に取るため、全員が片方だけ握った状態を作れば待ちグラフに
閉路ができます。
教科書的な資源階層（resource hierarchy）にはなっていません。

実際にデッドロックを避けているのは `philo_routine` 冒頭の 1 行、`if (philo->id % 2 == 0)
ft_usleep(1);` です。偶数 ID を 1 ms 遅らせ、奇数側と偶数側が交互にフォークを取りに来る
ように仕向けています。上の実行例が 1・3 → 2・4 と交互になっているのはこれが効いた結果です。
つまり保証ではなくタイミングによる非同期化で、証明ではなく観測でしか裏付けられません。
手元では N = 2〜5 について `800 200 200 10` を各 20 回、計 80 回実行してハング 0 回・
死亡 0 回でしたが、これはデッドロックしないことの証明ではありません。

### ft_usleep をポーリングで組む

素の `usleep` はスケジューリング粒度でオーバーシュートするため、500 µs 刻みで経過時間を
測り直しています（`get_current_time` は `gettimeofday` を `tv_sec * 1000 + tv_usec / 1000`
でミリ秒に畳んだもの）。実測（`./philo 5 800 200 200 20`、`time_to_eat = 200`、食事 98 回分の
「is eating」から「is sleeping」までの差分）は **中央値 201 ms、p90 218 ms、最大 264 ms**。
多くは指定値の数 ms 以内ですが、スケジューラの都合で数十 ms 伸びる回もあります。

### ファイル分割は Norm の制約が駆動している

42 の Norm は 1 ファイルにつき関数 5 個までと定めています。8 つの `.c`（`main` / `input` /
`init` / `threads` / `actions` / `monitor` / `utils` / `utils2`）への分割はこの上限と機能境界の
両方を満たすように切った結果で、`actions.c` と `utils.c` がちょうど 5 関数で上限に
張り付いています。構造体は `t_philo` と `t_program` の 2 つだけ。libft は持ち込まず必要な
4 関数（`ft_atoi` / `ft_isdigit` / `ft_strlen` / `ft_strcmp`）だけを自前で書いているので、
`philo/` 単体でビルドが完結します。

エラーは起動前に全部弾く方針で、`input.c` の 4 段階検証（引数個数 → 数字のみ → 正の整数 →
200 人以下）を 1 つでも落ちれば usage を出して `main` が `return (1)`。後始末の
`destroy_all` は「`str` が非 NULL ならメッセージ表示、NULL なら mutex 破棄」の兼用で、
Norm の関数数を増やさずに後始末を 1 本化しています。

## 既知の限界

- **`philo->eating` にデータ競合がある。** `eat()` は `meal_lock` の外で書き、
  `philosopher_dead()` は `meal_lock` の下で読みます。厳密には race であり、
  「完全に race-free」とは言えません。
- **デッドロックは構造的には排除されていない**（前述のとおり回避はタイミング依存）。
- **`ft_atoi` にオーバーフロー処理がない。** 数字以外と負号は弾きますが、`int` を超える
  桁数の入力は静かにラップします。
- **哲学者数の上限が 200 に固定**（静的配列の帰結）、**監視スレッドがスピンする**
  （アイドル時も CPU を使う）。
- **時間パラメータが厳しいと死亡しうる。** `think` に待機を入れていないため、食事の
  順番待ちに `time_to_die` の余裕がほとんどない設定では、スケジューラの巡り合わせ次第で
  死亡側に倒れます。負荷の高いマシンでは余裕を多めに取ってください。
- **ボーナス（プロセスとセマフォによる `philo_bonus`）は未実装。**

## 計測

手元の macOS（Apple Silicon）での実測値です。環境で変わります。

| 実行 | 結果 |
| --- | --- |
| `./philo 200 800 200 200 5` | wall 2.33 s / ログ 4700 行 / 死亡 0 |
| `./philo 5 800 200 200 20` | wall 10.7 s / ログ 495 行 / 死亡 0 |
| N = 2〜5 × `800 200 200 10` を各 20 回 | ハング 0 / 80、死亡 0 / 80 |
| `norminette philo/src philo/include` | 全 9 ファイル OK! |
| `make`（`-Wall -Wextra -Werror`） | 警告 0・エラー 0 |

## philo_annotated について

`philo_annotated/` は `philo/` と同じコードに日本語の解説コメントを全面的に付けた学習用の
コピーです（1679 行）。Norm は説明コメントを許さないので提出物はコメントゼロにせざるを
得ません。理解の記録を残しつつ提出物を汚さないために、物理的に別ディレクトリへ分けています。
コードが同一であることは `cc -fpreprocessed -dD -E -P` でコメントを除去して `cmp` で比較すれば
確認でき、8 ファイルすべて一致します。`philo_annotated/` 自体はコメント起因で norminette に
通りません。

## 学んだこと

- **共有状態は「守る対象」単位でロックを切る。** 1 本の巨大ロックでも動きますが、監視が
  走査している間ずっと出力が止まります。3 本に分けたことで、どのロックがどの不変条件を
  守っているかをコードを読まずに言えるようになりました。ロックの粒度は性能の話に見えて、
  実は設計を説明可能にするかどうかの話でした。
- **終了条件は 1 か所に持たせる。** 哲学者側に判断させると N 個のスレッドが同じ条件を
  各自の解釈で評価することになります。監視に集約したら終了処理の追跡が楽になりました。
- **「動いた」と「正しい」は別。** 偶数 ID を 1 ms ずらす実装は手元で何度回しても
  デッドロックしませんが、待ちグラフには閉路が残っています。正しさの証明ではなく確率を
  下げているだけで、この差を自分の言葉で説明できるようになったのが一番の収穫でした。
- **時間を扱うコードは実測しないと分からない。** `usleep` はスケジューリング粒度で
  オーバーシュートし、ポーリング実装にしてもなお最大 264 ms の回がありました。何を保証
  できて何を保証できないかは、測って初めて言えるようになりました。

## ライセンス

42 Tokyo のカリキュラム課題として作成した学習成果物です。
