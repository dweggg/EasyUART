class Database:
    def __init__(self):
        self.variables = []

    def add_variable(self, variable_name, baud_rate, data_type):
        # Example logic to limit variables based on baud rate and data types
        # This should be customized based on your requirements
        self.variables.append((variable_name, baud_rate, data_type))

    def get_variables(self):
        return self.variables

    # Additional methods to handle database operations can be added here.
