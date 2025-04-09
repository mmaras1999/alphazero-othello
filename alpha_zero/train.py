import os
import torch
import numpy as np
import pytorch_lightning as pl

from nn_model import AlphaZeroModel


def load_dataset(dataset_path: str | os.PathLike) -> torch.utils.data.Dataset:
    with open(os.path.join(dataset_path, 'board.bin'), 'rb') as data_file:
        x = torch.from_numpy(np.fromfile(data_file, dtype=np.float32)).view(-1, 3, 8, 8)
    
    with open(os.path.join(dataset_path, 'policy.bin'), 'rb') as data_file:
        policy = torch.from_numpy(np.fromfile(data_file, dtype=np.float32)).view(-1, 65)

    with open(os.path.join(dataset_path, 'value.bin'), 'rb') as data_file:
        value = torch.from_numpy(np.fromfile(data_file, dtype=np.float32)).view(-1, 1)

    return torch.utils.data.TensorDataset(x, policy, value)


class LitAlphaZero(pl.LightningModule):
    def __init__(self, model: torch.nn.Module):
        super().__init__()
        self.model = model
        self.value_loss_fn = torch.nn.MSELoss()
        self.policy_loss_fn = torch.nn.CrossEntropyLoss()

    def training_step(self, batch, batch_idx):
        x, policy, value = batch
        pred_value, pred_policy = self.model.forward_train(x)

        policy_loss = self.policy_loss_fn(pred_policy, policy)
        value_loss = self.value_loss_fn(pred_value, value)

        self.log('policy_loss', policy_loss)
        self.log('value_loss', value_loss)
        self.log('total_loss', policy_loss + value_loss, prog_bar=True)
        
        return policy_loss + value_loss

    def configure_optimizers(self):
        optimizer = torch.optim.AdamW(self.parameters(), lr=1e-3)
        scheduler = torch.optim.lr_scheduler.ReduceLROnPlateau(optimizer)
        return {"optimizer": optimizer, "lr_scheduler": scheduler, "monitor": "total_loss"}


if __name__ == "__main__":
    batch_size = 64

    dataset = load_dataset("datasets/iter_0")

    dataloader = torch.utils.data.DataLoader(
        dataset,
        batch_size,
        shuffle=True,
        pin_memory=True,
        num_workers=8,
    )

    model = AlphaZeroModel(256, 8)
    lit_module = LitAlphaZero(model)

    trainer = pl.Trainer(max_epochs=20)
    trainer.fit(model=lit_module, train_dataloaders=dataloader)

    onnx_model = lit_module.model.export_onnx()
    onnx_model.optimize()

    os.makedirs("models/", exist_ok=True)
    onnx_model.save("models/trained.onnx")
