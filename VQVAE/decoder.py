import torch
from torch import nn
from torch.nn import functional as F
from typing import List

from resnet_layer import ResidualLayer

class ConvDecoder(nn.Module):
    def __init__(self,
                 in_channels: int,
                 embedding_dim: int,
                 hidden_dims: List = [128, 256],
                 img_size: int = 32,
                 activation=nn.LeakyReLU,
                 **kwargs) -> None:
        super().__init__()
        
        # Build Decoder
        modules = []
        modules.append(
            nn.Sequential(
                nn.Conv2d(embedding_dim,
                          hidden_dims[-1],
                          kernel_size=1,
                          stride=1,
                          padding=0),
                activation())
        )

        for _ in range(6):
            modules.append(ResidualLayer(hidden_dims[-1], hidden_dims[-1]))

        modules.append(activation())

        hidden_dims.reverse()

        for i in range(len(hidden_dims) - 1):
            kernel_size = 3 if i == 0 else 4
            output_padding = 1 if i == 0 else 0
            modules.append(
                nn.Sequential(
                    nn.ConvTranspose2d(hidden_dims[i],
                                       hidden_dims[i + 1],
                                       kernel_size=kernel_size,
                                       stride=2,
                                       padding=1,
                                       output_padding=output_padding),
                    activation())
            )

        modules.append(
            nn.Sequential(
                nn.ConvTranspose2d(hidden_dims[-1],
                                   out_channels=in_channels,
                                   kernel_size=4,
                                   stride=2, 
                                   padding=1,
                                   output_padding=0),
                nn.Tanh()))

        self.decoder = nn.Sequential(*modules)
        
    def forward(self, x):
        return self.decoder(x)
