import sys

class Node:
    def __init__(self, name, **kwargs):
        self.name = name
        self.is_or = kwargs['isOr'] if 'isOr' in kwargs else False
        self.match_type = kwargs['type'] if 'type' in kwargs else "NONE"
        self.match_value = kwargs['value'] if 'value' in kwargs else ""
        self.children = []

    def add(self, child):
        self.children.append(child)

    def generateFSM(self):
        pass

    def generateFSMNode(self):


if __name__ == "__main__":
    list_head = Node("head")
    list_tail = Node("tail", isOr=True)
    value = Node("value", type="TOKEN")
    comma = Node("comma", type="SYMBOL", value=",")