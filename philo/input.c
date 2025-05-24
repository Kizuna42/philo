/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   input.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kizuna <kizuna@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/19 00:00:00 by student           #+#    #+#             */
/*   Updated: 2025/05/24 17:15:29 by kizuna           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	check_input(int argc, char **argv)
{
	if (argc != 5 && argc != 6)
	{
		printf("Error: Wrong number of arguments\n");
		printf("Usage: ./philo number_of_philosophers time_to_die ");
		printf("time_to_eat time_to_sleep ");
		printf("[number_of_times_each_philosopher_must_eat]\n");
		return (1);
	}
	if (check_valid_args(argv) == 1)
		return (1);
	return (0);
}

static int	check_digits(char *str)
{
	int	j;

	j = 0;
	while (str[j])
	{
		if (!ft_isdigit(str[j]))
		{
			printf("Error: Invalid argument '%s'\n", str);
			return (1);
		}
		j++;
	}
	return (0);
}

static int	check_values(char **argv, int i)
{
	if (ft_atoi(argv[i]) <= 0)
	{
		printf("Error: Arguments must be positive integers\n");
		return (1);
	}
	if (ft_atoi(argv[1]) > 200)
	{
		printf("Error: Number of philosophers cannot exceed 200\n");
		return (1);
	}
	return (0);
}

int	check_valid_args(char **argv)
{
	int	i;

	i = 1;
	while (argv[i])
	{
		if (check_digits(argv[i]) == 1)
			return (1);
		if (check_values(argv, i) == 1)
			return (1);
		i++;
	}
	return (0);
}
