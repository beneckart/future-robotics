import torch
from torch import nn
from torch.nn import functional as F
from typing import List

from resnet_layer import ResidualLayer

class ConvEncoder(nn.Module):
    def __init__(self,
                 in_channels: int,
                 embedding_dim: int,
                 hidden_dims: List = [128, 256],
                 activation=nn.LeakyReLU,
                 **kwargs) -> None:
        super().__init__()

        # Build Encoder
        modules = []
        for h_dim in hidden_dims:
            modules.append(
                nn.Sequential(
                    nn.Conv2d(in_channels, out_channels=h_dim,
                              kernel_size=4, stride=2, padding=1), # no batchnorm
                    activation())
            )
            in_channels = h_dim

        modules.append(
            nn.Sequential(
                nn.Conv2d(in_channels, in_channels,
                          kernel_size=3, stride=1, padding=1),
                activation())
        )

        for _ in range(6):
            modules.append(ResidualLayer(in_channels, in_channels))
        modules.append(activation())

        modules.append(
            nn.Sequential(
                nn.Conv2d(in_channels, embedding_dim,
                          kernel_size=1, stride=1),
                activation())
        )

        self.encoder = nn.Sequential(*modules)
        
    def forward(self, x):
        return self.encoder(x)
        

