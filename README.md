

---

# Perforce QAC License Diagnostic Tool

This program is designed to analyze a Perforce QAC license file, specifically focusing on the `diaglist` section. The `diaglist` section is always associated with subscription licenses and may expire. This tool helps you determine the number of days left before the license expires.

## Features

- **License File Analysis**: Parses the provided `.lic` file to determine the remaining days for the `diaglist` license.
- **Configuration File Support**: Saves the entered license file path in a `config.txt` file for future use.
- **Flexible Path Management**: Offers the option to update or delete the stored path and file name to work with a different license file.

## Usage

1. **Starting the Program**  
   When you run the program, it will prompt you to enter the path to your Perforce QAC license file.  
   Example:  
   ```plaintext
   Enter the license file path: c:\qaclic\prm-005056972d25-PO240194-240118.lic
   ```

2. **Analyzing the License File**  
   The program reads and analyzes the specified license file to calculate how many days are left for the `diaglist` license.

3. **Saving the Path**  
   - The entered path is stored in a `config.txt` file for convenience.  
   - On subsequent runs, the program will automatically use the path stored in `config.txt`.  
   - If `config.txt` is missing, the program will prompt you to enter a new path.

4. **Updating the Path**  
   - You can delete the stored path in `config.txt` and provide a new file path. This allows you to analyze a different license file.

## Handling Configuration File Issues

- If `config.txt` is accidentally deleted, the program will simply ask you to enter a new license file path.
- The tool is designed to handle such scenarios gracefully without affecting its functionality.

## Example Workflow

1. Run the program:
   ```plaintext
   Enter the license file path: c:\qaclic\prm-005056972d25-PO240194-240118.lic
   ```
2. The program analyzes the file and outputs:
   ```plaintext
   The diaglist license will expire in 45 days.
   ```
3. On subsequent runs, the program uses the stored path:
   ```plaintext
   Using saved path: c:\qaclic\prm-005056972d25-PO240194-240118.lic
   ```
4. To change the license file:
   - Choose the option to delete the stored path.
   - Provide a new file path when prompted.

## Prerequisites

- Ensure the Perforce QAC license file is accessible and contains a valid `diaglist` section.
- The program should be run in an environment with appropriate permissions to read/write files.

## Contributions

Contributions are welcome! Feel free to open an issue or submit a pull request to improve the tool.

## License

This project is licensed under the [MIT License](LICENSE).

---


