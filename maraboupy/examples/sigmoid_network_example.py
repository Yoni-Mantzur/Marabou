from MarabouNetwork import MarabouNetwork
from maraboupy import Marabou


filename = './networks/mnist.pb'
network = Marabou.read_tf(filename)  # type: MarabouNetwork


# Get the input and output variable numbers; [0] since first dimension is batch size
inputVars = network.inputVars[0][0]
outputVars = network.outputVars[0]

# # Set input bounds
for var in inputVars:
    network.setLowerBound(var, 0)
    network.setUpperBound(var, 1)

# Set output bounds
for var in outputVars:
    network.setLowerBound(var, 0)
    network.setUpperBound(var, 0)


# # Call to C++ Marabou solver
vals, stats = network.solve("marabou.log")