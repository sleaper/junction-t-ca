import pandas as pd
import matplotlib.pyplot as plt

density_df = pd.read_csv('results/flow_density_sym.csv')

# 0 aggressive drivers, varying density, symmetric rules + lane change
plt.figure(figsize=(14,6))
plt.subplot(1, 2, 1)
plt.plot(density_df['density'], density_df['flow'], 'o-')
plt.xlabel('Density')
plt.ylabel('Average Flow')
plt.title('Flow vs Density (Symmetric Rules)')
plt.grid(True)

plt.subplot(1, 2, 2)
plt.plot(density_df['density'], density_df['lane_change_rate'], 'o-', color='orange')
plt.xlabel('Density')
plt.ylabel('Lane Change Rate (changes/step)')
plt.title('Lane Change Rate vs Density (Symmetric Rules)')
plt.ylim(0, 0.005)
plt.grid(True)

plt.tight_layout()
plt.savefig('results/repro-symmetric.png')
plt.close()


# 0 aggressive drivers, varying density, asymmetric rules
asym_density_df = pd.read_csv('results/flow_density_asymmetric.csv')
plt.figure(figsize=(14, 6))

plt.subplot(1, 2, 1)
plt.plot(asym_density_df['density'], asym_density_df['flow'], 'bo-', label='Total Flow')
plt.plot(asym_density_df['density'], asym_density_df['left_flow'], 'cs-', label='Left Lane Flow')
plt.plot(asym_density_df['density'], asym_density_df['right_flow'], 'g^-', label='Right Lane Flow')
plt.xlabel('Density')
plt.ylabel('Average Flow')
plt.title('Flow vs Density (Asymmetric Rules)')
plt.legend()
plt.grid(True)

plt.subplot(1, 2, 2)
plt.plot(asym_density_df['density'], asym_density_df['lane_change_rate'], 'o-', color='orange')
plt.xlabel('Density')
plt.ylabel('Lane Change Rate')
plt.title('Lane Change Rate vs Density (Asymmetric Rules)')
plt.ylim(0, 0.005)
plt.grid(True)

plt.tight_layout()
plt.savefig('results/repro-asym.png')
plt.close()

# # varying aggressive driver ratio at low density
# aggressive_df = pd.read_csv('results/flow_aggressive_low_density.csv')
# plt.figure()
# plt.plot(aggressive_df['aggressive'], aggressive_df['flow'], 'o-')
# plt.xlabel('Aggressive Driver Ratio')
# plt.ylabel('Average Flow')
# plt.title('Flow vs Aggressive Driver Ratio at Low Density')
# plt.grid(True)
# plt.savefig('results/flow_vs_aggressive_driver_ratio.png')
# plt.close()

# # varying aggressive driver ratio at peak density
# peak_density_df = pd.read_csv('results/flow_aggressive.csv')
# plt.figure()
# plt.plot(peak_density_df['aggressive'], peak_density_df['flow'], 'o-')
# plt.xlabel('Aggressive Driver Ratio')
# plt.ylabel('Average Flow')
# plt.title('Flow vs Aggressive Driver Ratio at Peak Density')
# plt.grid(True)
# plt.savefig('results/flow_vs_aggressive_driver_ratio_peak_density.png')
# plt.close()

# # varying aggressive driver ratio at high density
# high_density_df = pd.read_csv('results/flow_aggressive_high_density.csv')
# plt.figure()
# plt.plot(high_density_df['aggressive'], high_density_df['flow'], 'o-')
# plt.xlabel('Aggressive Driver Ratio')
# plt.ylabel('Average Flow')
# plt.title('Flow vs Aggressive Driver Ratio at High Density')
# plt.grid(True)
# plt.savefig('results/flow_vs_aggressive_driver_ratio_high_density.png')
# plt.close()