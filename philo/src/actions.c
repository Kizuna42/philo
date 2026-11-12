/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   actions.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kizuna <kizuna@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/19 00:00:00 by student           #+#    #+#             */
/*   Updated: 2025/05/24 17:16:45 by kizuna           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static void	lock_fork(pthread_mutex_t *fork, t_philo *philo)
{
	pthread_mutex_lock(fork);
	print_message("has taken a fork", philo, philo->id);
}

void	take_forks(t_philo *philo)
{
	pthread_mutex_t	*first;
	pthread_mutex_t	*second;

	first = philo->r_fork;
	second = philo->l_fork;
	if (philo->id % 2 == 0)
	{
		first = philo->l_fork;
		second = philo->r_fork;
	}
	lock_fork(first, philo);
	if (philo->num_of_philos == 1)
	{
		while (!check_if_dead(philo))
			usleep(100);
		pthread_mutex_unlock(first);
		return ;
	}
	lock_fork(second, philo);
}

void	eat(t_philo *philo)
{
	take_forks(philo);
	if (philo->num_of_philos == 1)
		return ;
	pthread_mutex_lock(philo->meal_lock);
	philo->eating = 1;
	pthread_mutex_unlock(philo->meal_lock);
	print_message("is eating", philo, philo->id);
	pthread_mutex_lock(philo->meal_lock);
	philo->last_meal = get_current_time();
	philo->meals_eaten++;
	pthread_mutex_unlock(philo->meal_lock);
	ft_usleep(philo->time_to_eat);
	pthread_mutex_lock(philo->meal_lock);
	philo->eating = 0;
	pthread_mutex_unlock(philo->meal_lock);
	drop_forks(philo);
}

void	drop_forks(t_philo *philo)
{
	pthread_mutex_unlock(philo->l_fork);
	pthread_mutex_unlock(philo->r_fork);
}

void	philo_sleep(t_philo *philo)
{
	print_message("is sleeping", philo, philo->id);
	ft_usleep(philo->time_to_sleep);
}
