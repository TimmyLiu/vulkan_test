//#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <functional>
#include <vector>
#include <cstring>
#include <set>
#include <algorithm>

const int WIDTH = 800;
const int HEIGHT = 600;

const std::vector<const char*> validationLayers = {
"VK_LAYER_LUNARG_standard_validation"
};

const std::vector<const char*> deviceExtensions = {
VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct QueueFamilyIndices
{
	int graphicsFamily = -1;
	int presentFamily = -1;
	bool isComplete()
	{
		return ( 
			(graphicsFamily >= 0) &&
			(presentFamily >= 0)
			);
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

const bool enableValidationLayers = true;

VkResult CreateDebugReportCallbackEXT(
	VkInstance instance,
	const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
	const VkAllocationCallbacks *pAllocator,
	VkDebugReportCallbackEXT *pCallback
)
{
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr)
		return func(instance, pCreateInfo, pAllocator, pCallback);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;

}

class HelloTriangleApplication
{
public:
	void run()
	{
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}
protected:
private:
	void initVulkan()
	{
		createInstance();
		setupDebugCallback();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
	}

	void createSwapChain()
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		//
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}
		

	}

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;
		//capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &(details.capabilities));
		//formats
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}
		//present mode
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}


		return details;
	}

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
	{
		if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
			return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

		for (const auto &availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return availableFormat;
		}

		//settles with the first format
		return availableFormats[0];
	}

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
	{
		VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;
		for (const auto &availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				return VK_PRESENT_MODE_MAILBOX_KHR;
			else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
				bestMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
		return bestMode;
	}

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
			return capabilities.currentExtent;
		else
		{
			VkExtent2D actualExtent = { WIDTH, HEIGHT };
			actualExtent.width = std::max(capabilities.minImageExtent.width,
				std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height,
				std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}

	bool checkDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto &extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}
		return requiredExtensions.empty();
	}

	void createSurface()
	{
		VkResult err = glfwCreateWindowSurface(instance, window, nullptr, &surface);
		if (err != VK_SUCCESS)
			throw std::runtime_error("failed to create window surface");
		else
			std::cout << "successfully created window surface" << std::endl;
	}
	void createLogicalDevice()
	{
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		/*
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.graphicsFamily;
		queueCreateInfo.queueCount = 1;

		float queuePriority = 1.0f;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		*/
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfo_vec;
		std::vector<int> uniqueQueueFamily_vec = { indices.graphicsFamily,
			indices.presentFamily };
		float queuePriority = 1.0f;
		for (int queueFamily : uniqueQueueFamily_vec)
		{
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfo_vec.push_back(queueCreateInfo);
		}


		//create device
		VkPhysicalDeviceFeatures deviceFeatures = {};
		VkDeviceCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfo_vec.data();
		createInfo.queueCreateInfoCount = queueCreateInfo_vec.size();

		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = deviceExtensions.size();
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
			createInfo.enabledLayerCount = 0;

		VkResult err = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
		if (err != VK_SUCCESS)
			throw std::runtime_error("failed to create logical device");
		else
			std::cout << "successfully created logical device " << std::endl;

		vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue);

	}
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto &queueFamily : queueFamilies)
		{
			//check graphics support
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}
			//check presentation capability
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (queueFamily.queueCount > 0 && presentSupport)
				indices.presentFamily = i;

			if (indices.isComplete())
				break;
			i++;
		}

		return indices;
	}
	bool isDeviceSuitable(VkPhysicalDevice device)
	{
		bool bool_to_return = false;
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
			deviceFeatures.geometryShader)
			bool_to_return = true;
		else
			bool_to_return = false;

		QueueFamilyIndices indices = findQueueFamilies(device);

		bool extensionSupported = checkDeviceExtensionSupport(device);

		bool_to_return = bool_to_return && (indices.isComplete()) && extensionSupported;

		//check is swapChain good enough
		bool swapChainAdequate = false;
		if (extensionSupported)
		{
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = (!swapChainSupport.formats.empty()) &&
				(!swapChainSupport.presentModes.empty());
		}
		bool_to_return = bool_to_return && swapChainAdequate;
		return  bool_to_return;
	}
	void pickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount == 0)
			throw std::runtime_error("failed to find physical devices");
		else
			std::cout << "deviceCount = " << deviceCount << std::endl;

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		for (const auto &device : devices)
		{
			if (isDeviceSuitable(device))
			{
				physicalDevice = device;
				std::cout << "found a suitable device" << std::endl;
				break;
			}
		}
	}
	void mainLoop()
	{
		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
		}
	}
	void cleanup()
	{
		vkDestroyDevice(device, nullptr);
		DestroyDebugReportCallbackEXT(instance, callback, nullptr);
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);
		glfwDestroyWindow(window);
		glfwTerminate();
	}
	void initWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan window", nullptr, nullptr);
	}
	void createInstance()
	{
		if (enableValidationLayers && !checkValidationLayerSupport())
		{
			throw std::runtime_error("validation layer requested was not available.");
		}
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "hello triangle from Timmy";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		//unsigned int glfwExtensionCount = 0;
		//const char** glfwExtensions;
		//glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		auto extensions = getRequiredExtensions();

		createInfo.enabledExtensionCount = extensions.size();
		createInfo.ppEnabledExtensionNames = extensions.data();

		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		VkResult err = vkCreateInstance(&createInfo, nullptr, &instance);
		if (err != VK_SUCCESS)
			throw std::runtime_error("error at vkCreateInstance");
		else
			std::cout << "successfully created a vulkan instance" << std::endl;

	}

	bool checkValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers)
		{
			bool layerFound = false;
			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
				return false;
		}
		return true;
	}

	std::vector<const char*> getRequiredExtensions()
	{
		std::vector<const char*> extensions;

		unsigned int glfwExtensionsCount = 0;
		const char **glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

		for (unsigned int i = 0; i < glfwExtensionsCount; i++)
			extensions.push_back(glfwExtensions[i]);

		if (enableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}
		return extensions;
	}
	void setupDebugCallback()
	{
		if (!enableValidationLayers)
			return;

		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		createInfo.pfnCallback = debugCallback;

		VkResult err = CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback);
		if (err != VK_SUCCESS)
		{
			throw std::runtime_error("failed to set up debug callback");
		}
		else
			std::cout << "successfully set up debug callback" << std::endl;

	}
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t obj,
		size_t location,
		int32_t code,
		const char *layerPrefix,
		const char *msg,
		void *userData
	)
	{
		std::cerr << "validation layer: " << msg << std::endl;
		return VK_FALSE;
	}

	void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks *pAllocator)
	{
		auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
		if (func != nullptr)
			func(instance, callback, pAllocator);
	}

	/**/
	GLFWwindow *window;
	VkInstance instance;
	VkDebugReportCallbackEXT callback;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkSurfaceKHR surface;
};

int main()
{
	HelloTriangleApplication app;
	try {
		app.run();
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;

}
