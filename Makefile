play-game:
	g++ play_game.cpp -I lib/ -o play_game -O3

collect-dataset:
	g++ collect_dataset.cpp -pthread -I lib/ -o collect_dataset -O3 -mavx2
