/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kizuna <kizuna@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/19 00:00:00 by student           #+#    #+#             */
/*   Updated: 2025/05/24 17:20:49 by kizuna           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILO_H
# define PHILO_H

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <pthread.h>
# include <sys/time.h>

typedef struct s_philo
{
	pthread_t		thread;
	int				id;
	int				eating;
	int				meals_eaten;
	size_t			last_meal;
	size_t			time_to_die;
	size_t			time_to_eat;
	size_t			time_to_sleep;
	size_t			start_time;
	int				num_of_philos;
	int				num_times_to_eat;
	int				*dead;
	pthread_mutex_t	*r_fork;
	pthread_mutex_t	*l_fork;
	pthread_mutex_t	*write_lock;
	pthread_mutex_t	*dead_lock;
	pthread_mutex_t	*meal_lock;
}					t_philo;

typedef struct s_program
{
	int				dead_flag;
	pthread_mutex_t	dead_lock;
	pthread_mutex_t	meal_lock;
	pthread_mutex_t	write_lock;
	t_philo			*philos;
}					t_program;

// utils.c
int		ft_atoi(const char *str);
int		ft_isdigit(int c);
int		ft_strlen(const char *s);
size_t	get_current_time(void);
int		ft_usleep(size_t milliseconds);

// input.c
int		check_input(int argc, char **argv);
int		check_valid_args(char **argv);

// init.c
void	init_program(t_program *program, t_philo *philos);
void	init_forks(pthread_mutex_t *forks, int philo_num);
void	init_philos(t_philo *philos, t_program *program,
			pthread_mutex_t *forks, char **argv);

// threads.c
int		thread_create(t_program *program, pthread_mutex_t *forks);
void	*philo_routine(void *pointer);
void	*monitor(void *pointer);

// actions.c
void	take_forks(t_philo *philo);
void	eat(t_philo *philo);
void	drop_forks(t_philo *philo);
void	philo_sleep(t_philo *philo);
void	think(t_philo *philo);

// monitor.c
int		philosopher_dead(t_philo *philo, size_t time_to_die);
int		check_death_and_set_flag(t_philo *philos);
int		check_if_dead(t_philo *philos);
int		check_if_all_ate(t_philo *philos);

// utils2.c
void	print_message(char *str, t_philo *philo, int id);
int		ft_strcmp(char *s1, char *s2);
void	destroy_all(char *str, t_program *program, pthread_mutex_t *forks);

#endif
