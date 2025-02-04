#include <sourcemod>
#include <profiler>
#include <yyjson>

#pragma dynamic 531072
#define TEST_ITERATIONS 100

Profiler g_hProfiler;

public Plugin myinfo = {
	name = "YYJSON Benchmark",
	author = "ProjectSky",
	description = "Performance benchmark for YYJSON",
	version = "1.0.0",
	url = "https://github.com/ProjectSky/sm-ext-yyjson"
};

public void OnPluginStart()
{
	RegServerCmd("sm_yyjson_benchmark", Command_Benchmark, "Run YYJSON performance benchmark");

	g_hProfiler = new Profiler();
}

Action Command_Benchmark(int args)
{
	YYJSON json = YYJSON.Parse("twitter.json", true);

	int dataLength = json.ReadSize;

	char[] jsonStr = new char[dataLength];
	json.ToString(jsonStr, dataLength);
	
	PrintToServer("Test data size: %.2f MB", float(dataLength) / (1024.0 * 1024.0));
	
	g_hProfiler.Start();
	for (int i = 0; i < TEST_ITERATIONS; i++)
	{
		YYJSON testJson = YYJSON.Parse(jsonStr);
		delete testJson;
	}
	g_hProfiler.Stop();
	float parseTime = g_hProfiler.Time;
	
	g_hProfiler.Start();
	for (int i = 0; i < TEST_ITERATIONS; i++)
	{
		json.ToString(jsonStr, dataLength);
	}
	g_hProfiler.Stop();
	float stringifyTime = g_hProfiler.Time;

	float parseTimePerOp = parseTime * 1000.0 / TEST_ITERATIONS;
	float stringifyTimePerOp = stringifyTime * 1000.0 / TEST_ITERATIONS;
	
	PrintToServer("=== YYJSON Performance Benchmark ===");
	PrintToServer("Test iterations: %d", TEST_ITERATIONS);
	PrintToServer("Data size: %.2f MB", float(dataLength) / (1024.0 * 1024.0));
	PrintToServer("Parse time: %.3f seconds", parseTime);
	PrintToServer("Stringify time: %.3f seconds", stringifyTime);
	PrintToServer("Parse operations per second: %.2f ops/sec", 1000.0 / parseTimePerOp);
	
	float parseSpeed = float(dataLength) * TEST_ITERATIONS / (parseTime * 1024.0 * 1024.0);
	float stringifySpeed = float(dataLength) * TEST_ITERATIONS / (stringifyTime * 1024.0 * 1024.0);
	
	PrintToServer("Parse speed: %.2f MB/s (%.2f GB/s)", parseSpeed, parseSpeed / 1024.0);
	PrintToServer("Stringify speed: %.2f MB/s (%.2f GB/s)", stringifySpeed, stringifySpeed / 1024.0);
	PrintToServer("Stringify operations per second: %.2f ops/sec", 1000.0 / stringifyTimePerOp);
	PrintToServer("=== YYJSON Performance Benchmark End ===");
	
	delete json;
	return Plugin_Handled;
}