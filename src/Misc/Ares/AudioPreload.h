#pragma once

// Simple interface for async audio preloading
// This allows other parts of the code to queue audio files for background loading

namespace AudioPreload {
	// Queue an audio file for background preloading
	// This is safe to call from any thread
	void QueueFile(const char* filename);
	
	// Queue multiple audio files for preloading
	template<size_t N>
	void QueueFiles(const char* (&filenames)[N]) {
		for (const char* filename : filenames) {
			QueueFile(filename);
		}
	}
} 