import torch


class ResBlock(torch.nn.Module):
    def __init__(self, channels=256):
        super().__init__()
    
        self.layers = torch.nn.Sequential(
            torch.nn.Conv2d(in_channels=channels, out_channels=channels, kernel_size=3, padding=1, bias=False),
            torch.nn.BatchNorm2d(channels),
            torch.nn.ReLU(),
            torch.nn.Conv2d(in_channels=channels, out_channels=channels, kernel_size=3, padding=1, bias=False),
            torch.nn.BatchNorm2d(channels)
        )

    def forward(self, x):
        return torch.nn.functional.relu(self.layers(x) + x)


class AlphaZeroModel(torch.nn.Module):
    def __init__(self, channels=256, num_resnet=12):
        super().__init__()

        self.base_layers = torch.nn.Sequential(
            # initial conv layer
            torch.nn.Conv2d(in_channels=3, out_channels=channels, kernel_size=3, padding=1, bias=False),
            torch.nn.BatchNorm2d(channels),
            torch.nn.ReLU(),
            *[ResBlock(channels) for _ in range(num_resnet)],
        )

        self.value_head = torch.nn.Sequential(
            torch.nn.Conv2d(in_channels=channels, out_channels=1, kernel_size=1),
            torch.nn.ReLU(),
            torch.nn.Flatten(),
            torch.nn.Linear(64, 256),
            torch.nn.ReLU(),
            torch.nn.Linear(256, 1),
            torch.nn.Tanh(),
        )

        self.policy_head = torch.nn.Sequential(
            torch.nn.Conv2d(in_channels=channels, out_channels=1, kernel_size=1),
            torch.nn.ReLU(),
            torch.nn.Flatten(),
            torch.nn.Linear(64, 256),
            torch.nn.ReLU(),
            torch.nn.Linear(256, 65),
        )

    def forward(self, x):
        x = self.base_layers(x)
        return self.value_head(x), torch.nn.functional.softmax(self.policy_head(x), 1)
    
    def forward_train(self, x):
        x = self.base_layers(x)
        return self.value_head(x), self.policy_head(x)
    
    def export_onnx(self):
        input_tensor = torch.rand((1, 3, 8, 8), dtype=torch.float32)

        return torch.onnx.export(
            self,
            (input_tensor,),
            input_names=["input"],
            output_names=["value", "policy"],
            dynamo=True,
            dynamic_axes={
                'input' : {0 : 'batch_size'},
                'value' : {0 : 'batch_size'},
                'policy': {0 : 'batch_size'},
            },
        )
        


if __name__ == "__main__":
    import os

    model = AlphaZeroModel(num_resnet=8)

    # export empty model to ONNX
    onnx_model = model.export_onnx()
    onnx_model.optimize()

    os.makedirs("models/", exist_ok=True)
    onnx_model.save("models/trained.onnx")
