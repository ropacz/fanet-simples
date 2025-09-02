---
applyTo: '**'
---
Provide project context and coding guidelines that AI should follow when generating code, answering questions, or reviewing changes.

1. **Project Context**:
   - The project is a simulation of a Flying Ad-hoc Network (FANET) using OMNeT++ and INET.
   - The main components include UAVs (Unmanned Aerial Vehicles) and a Ground Control Station (GCS).
   - Communication between UAVs and the GCS is done via UDP sockets.
   - The application layer handles neighbor discovery, data transmission, and connectivity checks.
   - The distance between UAVs and the GCS is a critical factor in communication reliability.

**2. Coding Guidelines**
   - Follow project style and naming conventions.
   - Use clear, meaningful names for variables and functions.
   - Comment only complex logic or important decisions.
   - Manage memory properly (free allocations, avoid leaks).
   - Always close files and release resources.
   - Optimize performance only in critical sections.
   - Handle errors safely and validate all inputs.
   - Prefer simple, readable, and maintainable solutions.
   - Keep functions in a logical order.
   - Avoid duplicate functionalities.
   - Follow a clear architecture.
   - Ensure each function has a single responsibility.