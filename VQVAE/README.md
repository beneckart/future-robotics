
This is a sample VQ-VAE based on pytorch lightning.

Run in terminal: 
> python vq_vae.py --embedding_dim=64 --num_embeddings=4096

Then start tensorboard: 
> tensorboard --logdir ./lightning_logs
> 
Go to localhost:6006 to see status of training.

Try `python vq_vae.py --help` to see all options.
