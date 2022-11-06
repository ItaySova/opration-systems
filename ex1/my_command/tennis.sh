#!/bin/bash
# Itay Sova 313246209
player_1="player_1"
player_1_points=50
player_2="player_2"
player_2_points=50

turn_to_pick=1
game_on=true
ball_place=0
player_input () {
	# take user input and check, if not valid call itself recursively
	read -s number
	if [[ ! $number =~ ^[0-9]+$ ]] || [[ $number -gt $current_player_points ]]
	then 
		echo
		echo "NOT A VALID MOVE !"
		if [[ $turn_to_pick = 1 ]]
			then
				echo -n "PLAYER 1 PICK A NUMBER: "
			else
				echo -n "PLAYER 2 PICK A NUMBER: "
		fi
		player_input
	fi
	chosen_number=$number
}
player_pick(){
	# player 1 choose number
	current_player_points=$player_1_points
	echo -n "PLAYER 1 PICK A NUMBER: "
	turn_to_pick=1
	player_input
	player_1_number=$chosen_number
	echo #newline
	# player 2 choose number
	current_player_points=$player_2_points
	echo -n "PLAYER 2 PICK A NUMBER: "
	turn_to_pick=2
	player_input
	player_2_number=$chosen_number 
	echo #newline
  # decide who wins this turn
	compute_ball_place
	# update the points
	player_1_points=$[$player_1_points - $player_1_number]
	player_2_points=$[$player_2_points - $player_2_number]
}

compute_ball_place () {
	# compute the new ball place in the end of the turn
	if [[ $player_1_number -gt $player_2_number ]]
		then 
			if [[ $ball_place -ge 0 ]]
				then
					ball_place=$[$ball_place + 1]
			else
				ball_place=1
			fi
	fi
	if [[ $player_1_number -lt $player_2_number ]]
		then
			if [[ $ball_place -le 0 ]]
				then
					ball_place=$[$ball_place - 1]
			else
				ball_place=-1
			fi
	fi
}

check_game_state () {
	echo -e "       Player 1 played: ${player_1_number}\n       Player 2 played: ${player_2_number}\n\n"
	# check game state:
	# if the ball reached 3 player 1 won
	if [[ $ball_place -eq 3 ]]
		then
			echo "PLAYER 1 WINS !"
			game_on=false
	# if the ball reached -3 player 2 won
	elif [[ $ball_place -eq -3 ]]
		then
			echo "PLAYER 2 WINS !"
			game_on=false
	# if the ball is not in 3 or -3:
	else
		# if player 1 has no points and player 2 still has
		if [[ $player_1_points -eq 0 ]] && [[ $player_2_points -gt 0 ]]
			then
				echo "PLAYER 2 WINS !"
				game_on=false
		fi
		# if player 2 has no points and player 1 still has
		if [[ $player_2_points -eq 0 ]] && [[ $player_1_points -gt 0 ]]
			then
				echo "PLAYER 1 WINS !"
				game_on=false
		fi
		# if both players has 0 points left in the end of the turn
		if [[ $player_2_points -eq 0 ]] && [[ $player_1_points -eq 0 ]]
			then
				# if the ball is in the positive half of the court player 2 lost
				if [[ $ball_place -gt 0 ]]
					then
						echo "PLAYER 1 WINS !"
				fi
				# if the ball is in the negative half of the court player 1 lost
				if [[ $ball_place -lt 0 ]]
					then
						echo "PLAYER 2 WINS !"
				fi
				# if the ball is in the MIDDLE its a draw
				if [[ $ball_place -eq 0 ]]
					then
						echo "IT'S A DRAW !"
				fi
				# the game is over
				game_on=false
		fi
	fi
}

print_ball_place() {
	case $ball_place in
	"0")
		echo " |       |       O       |       | "
		;;
	"-1")
		echo " |       |   O   #       |       | "
		;;
	"-2")
		echo " |   O   |       #       |       | "
		;;
	"-3")
		echo "O|       |       #       |       | "
		;;
	"1")
		echo " |       |       #   O   |       | "
		;;
	"2")
		echo " |       |       #       |   O   | "
		;;
	"3")
		echo " |       |       #       |       |O"
		;;
	esac
}
print_board () {
	echo " Player 1: $player_1_points         Player 2: $player_2_points "
	echo " --------------------------------- "
	echo " |       |       #       |       | "
	echo " |       |       #       |       | "
	print_ball_place
	echo " |       |       #       |       | "
	echo " |       |       #       |       | "
	echo " --------------------------------- "
}
# the actual game:
print_board
# the main loof for funning the game
while $game_on
do
	player_pick
	print_board
	check_game_state
done
