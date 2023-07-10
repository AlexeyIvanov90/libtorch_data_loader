#pragma once

#include <torch/torch.h>
#include <opencv2/opencv.hpp>


struct ConvNetImpl : public torch::nn::Module 
{
    ConvNetImpl(int64_t channels, int64_t height, int64_t width) 
		: conv1(torch::nn::Conv2dOptions(3 /*input channels*/, 8 /*output channels*/, 3 /*kernel size*/).stride(1)),
		conv2(torch::nn::Conv2dOptions(8, 16, 3).stride(1)),
		conv3(torch::nn::Conv2dOptions(16, 16, 3).stride(1)),
		conv4(torch::nn::Conv2dOptions(16, 8, 3).stride(1)),
		n(GetConvOutput(channels, height, width)),
		lin1(n, 256),
		lin2(256, 256 /*number of output classes*/),
		lin3(256, 2)
	{

		register_module("conv1", conv1);
		register_module("conv2", conv2);

		register_module("conv3", conv3);
		register_module("conv4", conv4);

		register_module("lin1", lin1);
		register_module("lin2", lin2);
		register_module("lin3", lin3);
    };

    torch::Tensor forward(torch::Tensor x) {
		x = torch::relu(torch::max_pool2d(conv1(x), 2));
		x = torch::relu(torch::max_pool2d(conv2(x), 2));
		x = torch::relu(torch::max_pool2d(conv3(x), 2));

		x = torch::relu(torch::max_pool2d(conv4(x), 2));

		x = x.view({ -1, n });

		x = torch::relu(lin1(x));

		x = torch::relu(lin2(x));
		x = torch::log_softmax(lin3(x), 1/*dim*/);

		return x;
    };


	int64_t GetConvOutput(int64_t channels, int64_t height, int64_t width) {
		torch::Tensor x = torch::zeros({ 1, channels, height, width });
		x = torch::max_pool2d(conv1(x), 2);
		x = torch::max_pool2d(conv2(x), 2);
		x = torch::max_pool2d(conv3(x), 2);
		x = torch::max_pool2d(conv4(x), 2);
		return x.numel();
    }


	torch::nn::Conv2d conv1, conv2, conv3, conv4;
	int64_t n;
	torch::nn::Linear lin1, lin2, lin3;
};


TORCH_MODULE(ConvNet);


torch::Tensor classification(torch::Tensor img_tensor, ConvNet model);
double classification_accuracy(std::string file_csv, ConvNet model);
void train(std::string train_file_csv, std::string val_file_csv, ConvNet model, int epochs, torch::Device device = torch::kCPU);