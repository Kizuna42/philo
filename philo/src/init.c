/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kizuna <kizuna@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/19 00:00:00 by student           #+#    #+#             */
/*   Updated: 2025/05/24 17:16:07 by kizuna           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

void	init_program(t_program *program, t_philo *philos)
{
	program->dead_flag = 0;
	program->philos = philos;
	pthread_mutex_init(&program->dead_lock, NULL);
	pthread_mutex_init(&program->meal_lock, NULL);
	pthread_mutex_init(&program->write_lock, NULL);
}

void	init_forks(pthread_mutex_t *forks, int philo_num)
{
	int	i;

	i = 0;
	while (i < philo_num)
	{
		pthread_mutex_init(&forks[i], NULL);
		i++;
	}
}

static void	assign_forks(t_philo *philo, pthread_mutex_t *forks, int i)
{
	int	philo_num;

	philo_num = philo->num_of_philos;
	philo->l_fork = &forks[i];
	if (i == 0)
		philo->r_fork = &forks[philo_num - 1];
	else
		philo->r_fork = &forks[i - 1];
}

void	init_philos(t_philo *philos, t_program *program,
			pthread_mutex_t *forks, char **argv)
{
	int	i;

	i = 0;
	while (i < ft_atoi(argv[1]))
	{
		philos[i].id = i + 1;
		philos[i].eating = 0;
		philos[i].meals_eaten = 0;
		philos[i].start_time = get_current_time();
		philos[i].last_meal = get_current_time();
		philos[i].time_to_die = ft_atoi(argv[2]);
		philos[i].time_to_eat = ft_atoi(argv[3]);
		philos[i].time_to_sleep = ft_atoi(argv[4]);
		philos[i].num_of_philos = ft_atoi(argv[1]);
		if (argv[5])
			philos[i].num_times_to_eat = ft_atoi(argv[5]);
		else
			philos[i].num_times_to_eat = -1;
		philos[i].dead = &program->dead_flag;
		philos[i].write_lock = &program->write_lock;
		philos[i].dead_lock = &program->dead_lock;
		philos[i].meal_lock = &program->meal_lock;
		assign_forks(&philos[i], forks, i);
		i++;
	}
}
