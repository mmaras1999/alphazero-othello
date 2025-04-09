play-game:
	g++ play_game.cpp -I lib/ -o build/play_game -O3 -lonnxruntime

collect-dataset:
	g++ collect_dataset.cpp -pthread -I lib/ -o build/collect_dataset -O3 -mavx2 -lonnxruntime

run-loop: collect-dataset play-game
	bash simple_loop.sh
