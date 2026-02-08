import subprocess
import matplotlib.pyplot as plt
import os

ranges = [
    (10000, 50000),
    (50000, 100000),
    (100000, 500000),
    (500000,1000000),
    (1000000,2000000)
]
executable = "./fastprime"

def get_cpu_count():
    try:
        output = subprocess.check_output("lscpu | grep '^CPU(s):'", shell=True).decode()
        return int(output.split(':')[1].strip())
    except:
        return os.cpu_count()

def run_benchmark(rl, rh, max_procs):
    times = []
    procs = list(range(1, max_procs + 3))

    print(f"--- Benchmarking Range {rl} to {rh} ---")
    for n in procs:
        result = subprocess.run(
            [executable, str(rl), str(rh), str(n)],
            capture_output=True, text=True
        )
        try:
            time_taken = float(result.stdout.strip())
            times.append(time_taken)
            print(f"Procs: {n}, Time: {time_taken:.6f}s")
        except ValueError:
            print(f"Error parsing output for n={n}")
            times.append(0)
    return procs, times

def main():
    print("Compiling fastprime.c...")
    subprocess.run(["gcc", "fastprime.c", "-o", "fastprime", "-lm"])

    max_cpus = get_cpu_count()
    print(f"Detected {max_cpus} Logical CPUs.")

    plt.figure(figsize=(10, 6))

    for rl, rh in ranges:
        x, y = run_benchmark(rl, rh, max_cpus)
        plt.plot(x, y, marker='o', label=f'Range {rl}-{rh}')

    plt.title(f'Execution Time vs Number of Child Processes\n(Machine with {max_cpus} Logical Cores)')
    plt.xlabel('Number of Processes (n)')
    plt.ylabel('Execution Time (seconds)')
    plt.legend()
    plt.grid(True)
    plt.xticks(x)

    plt.savefig('execution_time_plot.png')
    print("\nPlot saved as 'execution_time_plot.png'")
    plt.show()

if __name__ == "__main__":
    main()
