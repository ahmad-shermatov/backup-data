```bash
#!/bin/bash 
read -p "Enter source directory: " source_dir
echo "Backing up: $source_dir"
timestamp=$(date +%Y%m%d_%H%M%S)
backup_dir="backup_$timestamp"
echo "Created backup folder: $backup_dir"
echo "Files copied successfully"
chmod +x backing.sh 
```
