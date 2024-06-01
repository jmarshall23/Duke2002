// MtrBuild.cpp
//

#include "precompiled.h"

void R_LoadImage(const char* name, byte** pic, int* width, int* height, ID_TIME_T* timestamp, bool makePowerOf2);
void R_WriteTGA(const char* filename, const byte* data, int width, int height, bool flipVertical);

/*
====================
ConvertToGrayscale
====================
*/
void ConvertToGrayscale(byte* data, int width, int height) {
	for (int i = 0; i < width * height * 4; i += 4) {
		byte r = data[i];
		byte g = data[i + 1];
		byte b = data[i + 2];
		byte gray = static_cast<byte>(0.299f * r + 0.587f * g + 0.114f * b);
		data[i] = data[i + 1] = data[i + 2] = gray;
	}
}

/*
====================
ApplyScharrOperator
====================
*/
void ApplyScharrOperator(byte* data, byte* outData, int width, int height) {
	for (int y = 1; y < height - 1; ++y) {
		for (int x = 1; x < width - 1; ++x) {
			int index = (y * width + x) * 4;

			int gx = -3 * data[index - 4] - 10 * data[index] - 3 * data[index + 4] +
				3 * data[index - 4 + 4 * width] + 10 * data[index + 4 * width] + 3 * data[index + 4 + 4 * width];

			int gy = -3 * data[index - 4 * width - 4] - 10 * data[index - 4 * width] - 3 * data[index - 4 * width + 4] +
				3 * data[index + 4 * width - 4] + 10 * data[index + 4 * width] + 3 * data[index + 4 * width + 4];

			int magnitude = static_cast<int>(sqrt(gx * gx + gy * gy));
			magnitude = idMath::ClampInt(0, 255, magnitude);

			outData[index] = outData[index + 1] = outData[index + 2] = magnitude;
			outData[index + 3] = 255;  // Alpha channel
		}
	}
}

/*
====================
ApplyGaussianBlur
====================
*/
void ApplyGaussianBlur(const byte* input, byte* output, int width, int height) {
	const int kernelSize = 5;
	const float kernel[kernelSize][kernelSize] = {
		{ 1,  4,  7,  4,  1 },
		{ 4, 16, 26, 16,  4 },
		{ 7, 26, 41, 26,  7 },
		{ 4, 16, 26, 16,  4 },
		{ 1,  4,  7,  4,  1 }
	};
	const float kernelSum = 273.0f;

	for (int y = 2; y < height - 2; ++y) {
		for (int x = 2; x < width - 2; ++x) {
			float sum = 0.0f;
			for (int ky = -2; ky <= 2; ++ky) {
				for (int kx = -2; kx <= 2; ++kx) {
					float value = input[((y + ky) * width + (x + kx)) * 4] * 0.299f +
						input[((y + ky) * width + (x + kx)) * 4 + 1] * 0.587f +
						input[((y + ky) * width + (x + kx)) * 4 + 2] * 0.114f;
					sum += value * kernel[ky + 2][kx + 2];
				}
			}
			output[(y * width + x) * 4] = sum / kernelSum;
			output[(y * width + x) * 4 + 1] = sum / kernelSum;
			output[(y * width + x) * 4 + 2] = sum / kernelSum;
			output[(y * width + x) * 4 + 3] = 255;
		}
	}
}

/*
====================
ApplySobelFilter
====================
*/
void ApplySobelFilter(const byte* input, float* gradientX, float* gradientY, int width, int height) {
	const int kernelSize = 3;
	const float sobelX[kernelSize][kernelSize] = {
		{ -1, 0, 1 },
		{ -2, 0, 2 },
		{ -1, 0, 1 }
	};
	const float sobelY[kernelSize][kernelSize] = {
		{ -1, -2, -1 },
		{ 0,  0,  0 },
		{ 1,  2,  1 }
	};

	for (int y = 1; y < height - 1; ++y) {
		for (int x = 1; x < width - 1; ++x) {
			float sumX = 0.0f;
			float sumY = 0.0f;
			for (int ky = -1; ky <= 1; ++ky) {
				for (int kx = -1; kx <= 1; ++kx) {
					float value = input[((y + ky) * width + (x + kx)) * 4];
					sumX += value * sobelX[ky + 1][kx + 1];
					sumY += value * sobelY[ky + 1][kx + 1];
				}
			}
			gradientX[y * width + x] = sumX;
			gradientY[y * width + x] = sumY;
		}
	}
}

/*
====================
dnCreateNormalMap
====================
*/
void dnCreateNormalMap(const byte* albedoData, int width, int height, byte* normalData) {
#if 0
	byte* blurredData = (byte*)malloc(width * height * 4); // RGBA format
	ApplyGaussianBlur(albedoData, blurredData, width, height);

	float* gradientX = (float*)malloc(width * height * sizeof(float));
	float* gradientY = (float*)malloc(width * height * sizeof(float));
	ApplySobelFilter(blurredData, gradientX, gradientY, width, height);

	for (int y = 1; y < height - 1; ++y) {
		for (int x = 1; x < width - 1; ++x) {
			float dX = gradientX[y * width + x];
			float dY = gradientY[y * width + x];

			idVec3 normal = idVec3(-dX, -dY, 1.0f);
			normal.Normalize();
			normalData[(y * width + x) * 4 + 0] = (byte)((normal.x * 0.5f + 0.5f) * 255);
			normalData[(y * width + x) * 4 + 1] = (byte)((normal.y * 0.5f + 0.5f) * 255);
			normalData[(y * width + x) * 4 + 2] = (byte)((normal.z * 0.5f + 0.5f) * 255);
			normalData[(y * width + x) * 4 + 3] = 255;
		}
	}

	free(blurredData);
	free(gradientX);
	free(gradientY);
#else
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			normalData[(y * width + x) * 4 + 0] = 128;
			normalData[(y * width + x) * 4 + 1] = 128;
			normalData[(y * width + x) * 4 + 2] = 255;
			normalData[(y * width + x) * 4 + 3] = 255;
		}
	}
#endif
}

/*
====================
dnGenerateSpecularMap
====================
*/
void dnGenerateSpecularMap(const char* albedoMapPath) {
	// Load the albedo image
	int width, height;
	byte* albedoData;
	R_LoadImage(albedoMapPath, &albedoData, &width, &height, nullptr, true);

	// Check if the image was loaded successfully
	if (!albedoData) {
		common->Warning("Failed to load albedo map: %s", albedoMapPath);
		return;
	}

	// Convert to grayscale
	ConvertToGrayscale(albedoData, width, height);

	// Allocate memory for the specular map
	byte* specularData = (byte*)Mem_Alloc(width * height * 4);

	// Apply the Scharr operator to compute the specular map
	ApplyScharrOperator(albedoData, specularData, width, height);

	// Save the specular map
	idStr fixedFilePath = albedoMapPath;
	fixedFilePath.StripFileExtension();
	idStr specularMapPath = va("%s_spec.tga", fixedFilePath.c_str());
	R_WriteTGA(specularMapPath, specularData, width, height, true);

	// Free allocated memory
	Mem_Free(albedoData);
	Mem_Free(specularData);
}

/*
====================
dnGenerateNormalMap
====================
*/
void dnGenerateNormalMap(const char* filePath) {
	byte* albedoData;
	int width, height;
	ID_TIME_T timestamp;
	R_LoadImage(filePath, &albedoData, &width, &height, &timestamp, false);
	if (albedoData == nullptr) {
		common->Warning("Failed to load albedo image: %s", filePath);
		return;
	}

	// Allocate memory for normal and specular maps
	byte* normalData = (byte*)malloc(width * height * 4); // RGBA format

	// Create normal and specular maps
	dnCreateNormalMap(albedoData, width, height, normalData);

	// Save the maps
	idStr fixedFilePath = filePath;
	fixedFilePath.StripFileExtension();
	idStr normalMapPath = va("%s_normal.tga", fixedFilePath.c_str());
	R_WriteTGA(normalMapPath, normalData, width, height, true);

	// Cleanup
	free(albedoData);
	free(normalData);
}


/*
===========
dnSaveMTRFile
===========
*/
void dnSaveMTRFile(const char* folderPath, const idList<idStr>& processedFiles) {
	// Extract folder name from the folder path
	idStr folderStr = folderPath;
	folderStr.BackSlashesToSlashes();
	int lastSlashIndex = folderStr.Last('/');
	idStr folderName = folderStr.Right(folderStr.Length() - lastSlashIndex - 1);

	idStr mtrFileName = va("materials/%s.mtr", folderName.c_str());
	idFile* mtrFile = fileSystem->OpenFileWrite(mtrFileName);
	if (!mtrFile) {
		common->Warning("Failed to create .mtr file: %s", mtrFileName.c_str());
		return;
	}

	for (int i = 0; i < processedFiles.Num(); ++i) {
		idStr albedoPath = processedFiles[i];
		idStr normalPath = va("%s_normal.tga", albedoPath.c_str());
		idStr specularPath = va("%s_specular.tga", albedoPath.c_str());

		mtrFile->Printf("material %s {\n", albedoPath.c_str());
		mtrFile->Printf("    diffusemap %s\n", albedoPath.c_str());
		mtrFile->Printf("    bumpmap %s\n", normalPath.c_str());
		mtrFile->Printf("    specularmap %s\n", specularPath.c_str());
		mtrFile->Printf("}\n");
	}

	fileSystem->CloseFile(mtrFile);
}

/*
====================
MaterialBuild_f
====================
*/
void MaterialBuild_f(const idCmdArgs& args) {
	if (args.Argc() != 2) {
		common->Printf("Usage: mtrbuild <folder>\n");
		return;
	}

	const char* folderPath = args.Argv(1);

	idFileList* fileList = fileSystem->ListFiles(folderPath, ".tga", true, false);
	idList<idStr> processedFiles;

	for (int i = 0; i < fileList->GetNumFiles(); ++i) {
		const char* filePath = va("%s/%s", folderPath, fileList->GetFile(i));
		dnGenerateNormalMap(filePath);
		dnGenerateSpecularMap(filePath);

		idStr filePathFixed = filePath;
		filePathFixed.StripFileExtension();

		processedFiles.Append(filePathFixed);
	}

	dnSaveMTRFile(folderPath, processedFiles);

	fileSystem->FreeFileList(fileList);
}