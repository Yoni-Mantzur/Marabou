#!/bin/bash
#SBATCH --mem=20g
#SBATCH -c 2
#SBATCH --time=2-0
#SBATCH --mail-user=yoniman30@gmail.com
#SBATCH --mail-type=END,FAIL,TIME_LIMIT


module load tensorflow


dir=/cs/labs/guykatz/yoni_mantzur/marabou/maraboupy/examples

cd $dir

python sigmoid_network_example.py
