package ija.ija2024.homework1.common;

public class EmptyNode extends GameNode {
    public EmptyNode(Position position) {
        super(position);
    }

    @Override
    public boolean isBulb() {
        return false;
    }

    @Override
    public boolean isLink() {
        return false;
    }

    @Override
    public boolean isPower() {
        return false;
    }

    @Override
    public void turn() {
    }

    @Override
    public boolean containsConnector(Side s) {
        return false;
    }
}