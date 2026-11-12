# philo

42 の課題「Philosophers」（食事する哲学者問題）の実装です。

- `philo/` : 提出用ソース
- `philo_annotated/` : 学習用に日本語コメントを付けた同等のソース
- `tests/run_tests.sh` : 簡易スモークテスト

## ビルド

```sh
make -C philo
```

## 実行

```sh
./philo/philo number_of_philosophers time_to_die time_to_eat time_to_sleep \
	[number_of_times_each_philosopher_must_eat]
```

- 引数はすべて 1 以上 `INT_MAX` 以下の整数（符号なし）
- 哲学者の人数は最大 200
- 時間の単位はミリ秒

例:

```sh
./philo/philo 5 800 200 200 7   # 全員が 7 回食べたら終了
./philo/philo 4 310 200 100     # 誰かが死ぬ
```

## テスト

```sh
make -C philo
tests/run_tests.sh philo/philo
```

引数エラー、死亡判定のタイミング、死亡せずに終了するケースなどを確認します。
CI（GitHub Actions）ではこれに加えて ThreadSanitizer によるデータ競合チェックと norminette を実行します。
