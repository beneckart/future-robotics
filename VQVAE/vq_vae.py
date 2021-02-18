import torch, os
import numpy as np
from torch import nn
from torch.nn import functional as F
from typing import List, Union, TypeVar
import pytorch_lightning as pl
from argparse import ArgumentParser
from torch.utils.data import DataLoader, random_split
from torchvision.datasets import MNIST
from torchvision import transforms
from torchvision.utils import save_image
import encoder, decoder, vector_quantizer

Tensor = TypeVar('torch.tensor')

class VQVAE(pl.LightningModule):

    def __init__(self,
                 in_channels: int,
                 embedding_dim: int,
                 num_embeddings: int,
                 hidden_dims: List = [128,256],
                 beta: float = 0.25,
                 img_size: int = 64,
                 learning_rate: float = 1e-4,
                 batch_size: int = 16) -> None:
        super(VQVAE, self).__init__()
        self.save_hyperparameters()

        self.encoder = encoder.ConvEncoder(in_channels=in_channels,
                                           embedding_dim=embedding_dim,
                                           hidden_dims=hidden_dims,
                                           activation=nn.LeakyReLU)
                                           
        self.decoder = decoder.ConvDecoder(in_channels=in_channels,
                                           embedding_dim=embedding_dim,
                                           hidden_dims=hidden_dims,
                                           activation=nn.LeakyReLU)

        self.vq_layer = vector_quantizer.VectorQuantizer(num_embeddings,
                                                         embedding_dim,
                                                         self.hparams.beta)
                                        
        self.reconstruction_loss = nn.MSELoss(reduction='mean')
        self.embedding_loss = nn.MSELoss(reduction='mean')
        self.commitment_loss = nn.MSELoss(reduction='mean')

        self.numPlot = 0 # counter for making figures

    def training_step(self, batch, batch_idx):
        loss, encoding, quantized_encoding, quantized_encoding_residue, x_reconstructed, x = self._step(batch, batch_idx, prefix='train')
        return loss
        
    def test_step(self, batch, batch_idx):
        loss, encoding, quantized_encoding, quantized_encoding_residue, x_reconstructed, x = self._step(batch, batch_idx, prefix='test')
        return loss

    def validation_step(self, batch, batch_idx):
        loss, encoding, quantized_encoding, quantized_encoding_residue, x_reconstructed, x = self._step(batch, batch_idx, prefix='val')

        x_nonquantized_reconstructed = self.decoder(encoding) 
        
        R = np.prod(list(encoding.shape[2:])) # B x D x R1 x R2
       
        if batch_idx == 0:
            str_id = './saved_images_K%05d_D%04d_R%05d/' % (self.hparams.num_embeddings, self.hparams.embedding_dim, R)
            os.makedirs(str_id, exist_ok=True)
            save_image(x_reconstructed.view(-1, 1, 32, 32), '%s/%03d_decoded.png' % (str_id, self.numPlot), nrow=4)
            save_image(x.view(-1, 1, 32, 32), '%s/%03d_gt.png' % (str_id, self.numPlot), nrow=4)
            save_image(x_nonquantized_reconstructed.view(-1, 1, 32, 32), '%s/%03d_decoded_noquant.png' % (str_id, self.numPlot), nrow=4)
            self.numPlot += 1  
        
        return loss
        
    def validation_epoch_end(self, saved_outputs):
        pass ## TODO make a pretty figure showing some digits sampled from latent space
          
    def _step(self, batch, batch_idx, prefix):
    
        x, _ = batch
        encoding = self.encoder(x)
        quantized_encoding = self.vq_layer(encoding)
        
        # Add the residue back to the latents 
        # TODO BE: why? this is equivalent in value to quantized_encoding, but gradient info only on encoding part
        quantized_encoding_residue = encoding + (quantized_encoding - encoding).detach()
        
        x_reconstructed = self.decoder(quantized_encoding_residue)
        
        commitment_loss = self.commitment_loss(quantized_encoding.detach(), encoding)
        embedding_loss = self.embedding_loss(quantized_encoding, encoding.detach())
       
        recon_loss = self.reconstruction_loss(x_reconstructed, x)
        
        loss = recon_loss + embedding_loss + self.hparams.beta*commitment_loss

        self.log(f'{prefix}/loss', loss)
        self.log(f'{prefix}/reconstruction_loss', recon_loss)
        self.log(f'{prefix}/commitment_loss', self.hparams.beta*commitment_loss)
        self.log(f'{prefix}/embedding_loss', embedding_loss)
        
        return loss, encoding, quantized_encoding, quantized_encoding_residue, x_reconstructed, x
        
    def configure_optimizers(self):
        return torch.optim.Adam(self.parameters(), lr=self.hparams.learning_rate)

    @staticmethod
    def add_model_specific_args(parent_parser):
        parser = ArgumentParser(parents=[parent_parser], add_help=False)
        parser.add_argument('--embedding_dim', type=int, default=64)
        parser.add_argument('--num_embeddings', type=int, default=64)
        parser.add_argument('--learning_rate', type=float, default=1e-4)
        return parser


    def forward(self, x: Tensor, **kwargs) -> Tensor:
        ''' run autoencoding pipeline to recreate input data '''
        encoding = self.encoder(x)
        quantized_encoding = self.vq_layer(encoding)
        quantized_encoding_residue = encoding + (quantized_encoding - encoding).detach()
        x_reconstructed = self.decoder(quantized_encoding_residue) 
        return x_reconstructed

    def sample(self,
               num_samples: int,
               current_device: Union[int, str], **kwargs) -> Tensor:
        raise Warning('VQVAE sampler is not implemented.') # TODO
        
    
def cli_main():
    pl.seed_everything(42)

    # ------------
    # args
    # ------------
    parser = ArgumentParser()
    parser.add_argument('--batch_size', default=16, type=int)
    parser.add_argument('--num_workers', default=16, type=int)
    parser = pl.Trainer.add_argparse_args(parser) # get lightning-specific commandline options
    parser = VQVAE.add_model_specific_args(parser) # get model-defined commandline options
    args = parser.parse_args()
    
    # ------------
    # data
    # ------------
    
    #transform = transforms.Compose([transforms.ToTensor()]) #transforms.Grayscale(3), 
    transform = transforms.Compose([transforms.Resize((32,32)), transforms.ToTensor(), transforms.Normalize((0.1307,), (0.3081,))])
    dataset = MNIST('', train=True, download=True, transform=transform)#transforms.ToTensor())
    mnist_train, mnist_val = random_split(dataset, [55000, 5000])
    mnist_test = MNIST('', train=False, download=True, transform=transform) #transforms.ToTensor())

    train_loader = DataLoader(mnist_train, batch_size=args.batch_size, num_workers=args.num_workers, shuffle=True)
    val_loader = DataLoader(mnist_val, batch_size=args.batch_size, num_workers=args.num_workers)
    test_loader = DataLoader(mnist_test, batch_size=args.batch_size, num_workers=args.num_workers)

    # ------------
    # model
    # ------------
    
    vqvae = VQVAE(in_channels=1,
                  img_size=32,
                  embedding_dim=args.embedding_dim, 
                  num_embeddings=args.num_embeddings,
                  learning_rate=args.learning_rate, 
                  hidden_dims=[32,64,128,256,256], 
                  batch_size=args.batch_size) 
   
    print(vqvae)
    from torchsummary import summary
    summary(vqvae.to('cuda:0'), input_size=(1, 32, 32))

    # ------------
    # training
    # ------------
    trainer = pl.Trainer.from_argparse_args(args, gpus=1, progress_bar_refresh_rate=5, max_epochs=100)
    trainer.fit(vqvae, train_loader, val_loader)

    # ------------
    # testing
    # ------------
    result = trainer.test(test_dataloaders=test_loader)
    print(result)


if __name__ == '__main__':
    cli_main()
        
