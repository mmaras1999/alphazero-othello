# initiate empty model
python3 alpha_zero/nn_model.py

# infinite loop
while :
do
    ./build/collect_dataset
    python3 alpha_zero/train.py
    ./build/play_game
done