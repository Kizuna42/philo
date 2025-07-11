/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kizuna <kizuna@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/19 00:00:00 by student           #+#    #+#             */
/*   Updated: 2025/05/27 18:50:26 by kizuna           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/*
 * ============================================================================
 * MAIN.C - プログラムのエントリーポイント
 * ============================================================================
 *
 * このファイルは哲学者問題シミュレーションのメイン関数を含んでいます。
 * プログラムの実行フローは以下の通りです：
 *
 * 1. コマンドライン引数の検証
 * 2. プログラム構造体の初期化
 * 3. フォーク（mutex）の初期化
 * 4. 哲学者構造体の初期化
 * 5. スレッドの作成と実行
 * 6. リソースの解放
 *
 * 使用方法：
 * ./philo [哲学者数] [死亡時間] [食事時間] [睡眠時間] [食事回数(オプション)]
 *
 * 例：./philo 5 800 200 200 7
 * ============================================================================
 */

#include "philo.h"

/*
 * ============================================================================
 * main関数 - プログラムのエントリーポイント
 * ============================================================================
 *
 * 引数:
 *   argc: コマンドライン引数の数
 *   argv: コマンドライン引数の配列
 *         argv[1]: 哲学者の数 (1-200)
 *         argv[2]: 死亡までの時間 (ミリ秒)
 *         argv[3]: 食事にかかる時間 (ミリ秒)
 *         argv[4]: 睡眠時間 (ミリ秒)
 *         argv[5]: 各哲学者が食べる回数 (オプション、省略時は無制限)
 *
 * 戻り値:
 *   0: 正常終了
 *   1: エラー終了（引数が不正など）
 *
 * 処理の流れ:
 *   1. 引数チェック
 *   2. 各種初期化
 *   3. スレッド実行
 *   4. リソース解放
 * ============================================================================
 */
int	main(int argc, char **argv)
{
	t_program		program;        // プログラム全体の状態を管理する構造体
	t_philo			philos[200];    // 哲学者の配列（最大200人まで対応）
	pthread_mutex_t	forks[200];     // フォークのmutex配列（哲学者数と同じ）

	/* ステップ1: コマンドライン引数の検証 */
	if (check_input(argc, argv) == 1)
		return (1);                 // 引数が不正な場合はエラー終了

	/* ステップ2: プログラム構造体の初期化 */
	init_program(&program, philos);

	/* ステップ3: フォーク（mutex）の初期化 */
	init_forks(forks, ft_atoi(argv[1]));

	/* ステップ4: 哲学者構造体の初期化 */
	init_philos(philos, &program, forks, argv);

	/* ステップ5: スレッドの作成と実行開始 */
	thread_create(&program, forks);

	/* ステップ6: 全リソースの解放とクリーンアップ */
	destroy_all(NULL, &program, forks);

	return (0);                     // 正常終了
}
